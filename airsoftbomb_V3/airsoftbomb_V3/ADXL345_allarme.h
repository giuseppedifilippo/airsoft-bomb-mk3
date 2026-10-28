/*
 * ADXL345_allarme.h
 * ---------------------------------------------------------
 * Una sola funzione, controllaADXL345(), da chiamare a ogni giro di loop().
 *
 * - Al primo richiamo inizializza da sola l'ADXL345 (I2C) e,
 *   se richiesto, lo calibra (circa 1 secondo, sensore FERMO).
 * - Dopo e' NON bloccante: legge il sensore ogni 10 ms usando millis(),
 *   senza delay(), quindi non rallenta il resto del tuo codice.
 * - Manda un avviso sul monitor seriale solo quando l'accelerazione
 *   su un asse resta CONTINUAMENTE sopra la soglia per almeno
 *   durataMinMs millisecondi. Un avviso per evento.
 * - Restituisce true solo nel giro in cui scatta un nuovo avviso,
 *   cosi' il tuo codice puo' reagire (LED, buzzer, ecc.).
 *
 * Requisiti: Serial.begin(...) deve essere gia' stato chiamato
 * nel setup() del tuo progetto.
 *
 * Collegamenti I2C:
 *   Uno/Nano: SDA -> A4, SCL -> A5
 *   Mega:     SDA -> 20, SCL -> 21
 *   CS -> 3.3V, SDO -> GND (indirizzo 0x53) oppure SDO -> 3.3V (0x1D)
 */

#ifndef ADXL345_ALLARME_H
#define ADXL345_ALLARME_H

#include <Arduino.h>
#include <Wire.h>

/*
 * Parametri:
 *   sogliaG         soglia in g (1 g = 9.81 m/s^2)
 *   durataMinMs     tempo minimo continuo sopra soglia prima dell'avviso
 *   compensaGravita true  = sottrae la posizione a riposo misurata all'avvio
 *                           (la soglia vale per la variazione di accelerazione)
 *                   false = confronta l'accelerazione assoluta (gravita' inclusa)
 *   indirizzo       indirizzo I2C del sensore (0x53 o 0x1D)
 *
 * I parametri vengono letti a ogni chiamata, tranne compensaGravita
 * e indirizzo che contano solo alla prima (inizializzazione).
 */
bool controllaADXL345(float sogliaG = 1.5,
                      unsigned long durataMinMs = 200,
                      bool compensaGravita = true,
                      uint8_t indirizzo = 0x53)
{
  // ---------- Costanti ----------
  const uint8_t REG_DEVID       = 0x00;
  const uint8_t REG_BW_RATE     = 0x2C;
  const uint8_t REG_POWER_CTL   = 0x2D;
  const uint8_t REG_DATA_FORMAT = 0x31;
  const uint8_t REG_DATAX0      = 0x32;
  const float   SCALA_G         = 0.0039;     // 3.9 mg/LSB in full resolution
  const unsigned long PERIODO_LETTURA_MS = 10; // ~100 letture al secondo
  const char    NOMI_ASSI[3]    = {'X', 'Y', 'Z'};

  // ---------- Stato interno (mantenuto tra una chiamata e l'altra) ----------
  enum { DA_INIZIALIZZARE, PRONTO, ERRORE };
  static uint8_t stato = DA_INIZIALIZZARE;
  static float offset[3]               = {0, 0, 0};
  static bool sopraSoglia[3]           = {false, false, false};
  static unsigned long inizioSopra[3]  = {0, 0, 0};
  static bool avvisoInviato[3]         = {false, false, false};
  static float piccoMax[3]             = {0, 0, 0};
  static unsigned long ultimaLettura   = 0;

  // ---------- Funzioni di supporto (locali, non sporcano il tuo codice) ----------
  auto scriviRegistro = [indirizzo](uint8_t reg, uint8_t valore) {
    Wire.beginTransmission(indirizzo);
    Wire.write(reg);
    Wire.write(valore);
    Wire.endTransmission();
  };

  auto leggiRegistro = [indirizzo](uint8_t reg) -> uint8_t {
    Wire.beginTransmission(indirizzo);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return 0;
    Wire.requestFrom(indirizzo, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0;
  };

  // Legge i tre assi in g; restituisce false se la lettura I2C fallisce
  auto leggiAssi = [indirizzo, REG_DATAX0, SCALA_G](float v[3]) -> bool {
    Wire.beginTransmission(indirizzo);
    Wire.write(REG_DATAX0);
    if (Wire.endTransmission(false) != 0) return false;
    Wire.requestFrom(indirizzo, (uint8_t)6);
    if (Wire.available() < 6) return false;

    uint8_t b[6];
    for (int i = 0; i < 6; i++) b[i] = Wire.read();   // ordine garantito
    for (int i = 0; i < 3; i++) {
      int16_t raw = (int16_t)((uint16_t)b[2 * i] | ((uint16_t)b[2 * i + 1] << 8));
      v[i] = raw * SCALA_G;
    }
    return true;
  };

  // ---------- Inizializzazione (solo alla prima chiamata) ----------
  if (stato == ERRORE) return false;

  if (stato == DA_INIZIALIZZARE) {
    Wire.begin();

    uint8_t id = leggiRegistro(REG_DEVID);
    if (id != 0xE5) {
      Serial.print(F("ERRORE: ADXL345 non trovato (DEVID=0x"));
      Serial.print(id, HEX);
      Serial.println(F("). Controlla collegamenti e indirizzo."));
      stato = ERRORE;
      return false;
    }

    scriviRegistro(REG_BW_RATE, 0x0A);     // 100 Hz
    scriviRegistro(REG_DATA_FORMAT, 0x0B); // full resolution, +-16 g
    scriviRegistro(REG_POWER_CTL, 0x08);   // avvia le misure
    delay(20);

    if (compensaGravita) {
      Serial.println(F("ADXL345: calibrazione, tieni il sensore FERMO..."));
      delay(500);
      float somma[3] = {0, 0, 0}, v[3];
      int valide = 0;
      for (int n = 0; n < 50; n++) {
        if (leggiAssi(v)) {
          for (int i = 0; i < 3; i++) somma[i] += v[i];
          valide++;
        }
        delay(10);
      }
      if (valide > 0) {
        for (int i = 0; i < 3; i++) offset[i] = somma[i] / valide;
      }
    }

    Serial.print(F("ADXL345 pronto. Soglia "));
    Serial.print(sogliaG, 2);
    Serial.print(F(" g per almeno "));
    Serial.print(durataMinMs);
    Serial.println(F(" ms"));

    stato = PRONTO;
    ultimaLettura = millis();
    return false;
  }

  // ---------- Lettura periodica non bloccante ----------
  unsigned long adesso = millis();
  if (adesso - ultimaLettura < PERIODO_LETTURA_MS) return false;
  ultimaLettura = adesso;

  float v[3];
  if (!leggiAssi(v)) return false;
  for (int i = 0; i < 3; i++) v[i] -= offset[i];  // offset = 0 se non compensato

  // ---------- Controllo soglia + durata su ogni asse ----------
  bool nuovoAvviso = false;

  for (int i = 0; i < 3; i++) {
    float modulo = fabs(v[i]);

    if (modulo > sogliaG) {
      if (!sopraSoglia[i]) {
        // Inizio di un superamento: parte il timer
        sopraSoglia[i]   = true;
        inizioSopra[i]   = adesso;
        avvisoInviato[i] = false;
        piccoMax[i]      = modulo;
      } else if (modulo > piccoMax[i]) {
        piccoMax[i] = modulo;
      }

      // Sopra soglia da abbastanza tempo: avviso (una volta per evento)
      if (!avvisoInviato[i] && adesso - inizioSopra[i] >= durataMinMs) {
        avvisoInviato[i] = true;
        nuovoAvviso = true;
        Serial.print(F("!!! AVVISO: asse "));
        Serial.print(NOMI_ASSI[i]);
        Serial.print(F(" sopra soglia da "));
        Serial.print(adesso - inizioSopra[i]);
        Serial.print(F(" ms | X="));  Serial.print(v[0], 2);
        Serial.print(F(" Y="));       Serial.print(v[1], 2);
        Serial.print(F(" Z="));       Serial.print(v[2], 2);
        Serial.println(F(" g"));
      }
    } else if (sopraSoglia[i]) {
      // Tornato sotto soglia: fine evento
      if (avvisoInviato[i]) {
        Serial.print(F("    Fine evento asse "));
        Serial.print(NOMI_ASSI[i]);
        Serial.print(F(": durata "));
        Serial.print(adesso - inizioSopra[i]);
        Serial.print(F(" ms, picco "));
        Serial.print(piccoMax[i], 2);
        Serial.println(F(" g"));
      }
      // Se e' sceso prima di durataMinMs era un picco breve: ignorato
      sopraSoglia[i] = false;
    }
  }

  return nuovoAvviso;
}

#endif
