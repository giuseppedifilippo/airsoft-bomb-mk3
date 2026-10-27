/*
 * ADXL345 - Avviso su superamento soglia di accelerazione
 * ---------------------------------------------------------
 * Legge l'accelerometro via I2C (senza librerie esterne, solo Wire)
 * e stampa un avviso sul monitor seriale quando l'accelerazione
 * su un qualsiasi asse (X, Y o Z) supera la soglia impostata.
 *
 * Collegamenti (Arduino Uno/Nano):
 *   ADXL345 VCC -> 3.3V
 *   ADXL345 GND -> GND
 *   ADXL345 SDA -> A4
 *   ADXL345 SCL -> A5
 *   ADXL345 CS  -> 3.3V (abilita la modalita' I2C)
 *   ADXL345 SDO -> GND  (indirizzo 0x53; se SDO e' a 3.3V usa 0x1D)
 *
 * Nota: se il tuo modulo non ha regolatore/level shifter,
 * l'ADXL345 non tollera 5V sui pin di segnale.
 */

#include <Wire.h>

// ===================== CONFIGURAZIONE =====================
const uint8_t ADXL345_ADDR = 0x53;      // 0x1D se SDO e' collegato a VCC

const float SOGLIA_G = 1.5;             // soglia in g (1 g = 9.81 m/s^2)

// Se true, all'avvio misura la posizione a riposo e la sottrae:
// cosi' la soglia si applica alla VARIAZIONE di accelerazione
// (la gravita' sull'asse Z non fa scattare falsi allarmi).
// Se false, si confronta l'accelerazione assoluta (gravita' inclusa).
const bool COMPENSA_GRAVITA = true;

const unsigned long PAUSA_AVVISI_MS = 500;  // tempo minimo tra due avvisi
const unsigned long PERIODO_LETTURA_MS = 10; // ~100 letture al secondo
// ==========================================================

// Registri dell'ADXL345
const uint8_t REG_DEVID       = 0x00;
const uint8_t REG_BW_RATE     = 0x2C;
const uint8_t REG_POWER_CTL   = 0x2D;
const uint8_t REG_DATA_FORMAT = 0x31;
const uint8_t REG_DATAX0      = 0x32;

// In modalita' full resolution la sensibilita' e' sempre 3.9 mg/LSB
const float SCALA_G = 0.0039;

float offsetX = 0, offsetY = 0, offsetZ = 0;
unsigned long ultimoAvviso = 0;

// ---------- Funzioni I2C di base ----------
void scriviRegistro(uint8_t reg, uint8_t valore) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.write(valore);
  Wire.endTransmission();
}

uint8_t leggiRegistro(uint8_t reg) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);           // repeated start
  Wire.requestFrom(ADXL345_ADDR, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

// Legge i tre assi in g (lettura burst dei 6 byte di dati)
bool leggiAccelerazione(float &x, float &y, float &z) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(REG_DATAX0);
  if (Wire.endTransmission(false) != 0) return false;

  Wire.requestFrom(ADXL345_ADDR, (uint8_t)6);
  if (Wire.available() < 6) return false;

  // Dati little-endian, complemento a 2
  int16_t rawX = (int16_t)(Wire.read() | (Wire.read() << 8));
  int16_t rawY = (int16_t)(Wire.read() | (Wire.read() << 8));
  int16_t rawZ = (int16_t)(Wire.read() | (Wire.read() << 8));

  x = rawX * SCALA_G;
  y = rawY * SCALA_G;
  z = rawZ * SCALA_G;
  return true;
}

// Media di alcune letture a sensore fermo
void calibra() {
  const int N = 100;
  float sx = 0, sy = 0, sz = 0, x, y, z;
  int valide = 0;

  Serial.println(F("Calibrazione: tieni il sensore FERMO..."));
  delay(500);
  for (int i = 0; i < N; i++) {
    if (leggiAccelerazione(x, y, z)) {
      sx += x; sy += y; sz += z;
      valide++;
    }
    delay(10);
  }
  if (valide > 0) {
    offsetX = sx / valide;
    offsetY = sy / valide;
    offsetZ = sz / valide;
  }
  Serial.print(F("Offset (g): X="));  Serial.print(offsetX, 3);
  Serial.print(F(" Y="));             Serial.print(offsetY, 3);
  Serial.print(F(" Z="));             Serial.println(offsetZ, 3);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }                    // utile su Leonardo/Micro
  Wire.begin();

  // Verifica che il sensore risponda (DEVID deve valere 0xE5)
  uint8_t id = leggiRegistro(REG_DEVID);
  if (id != 0xE5) {
    Serial.print(F("ERRORE: ADXL345 non trovato (DEVID=0x"));
    Serial.print(id, HEX);
    Serial.println(F("). Controlla collegamenti e indirizzo."));
    while (true) { }
  }

  scriviRegistro(REG_BW_RATE, 0x0A);     // 100 Hz di output
  scriviRegistro(REG_DATA_FORMAT, 0x0B); // full resolution, range +-16 g
  scriviRegistro(REG_POWER_CTL, 0x08);   // avvia le misure
  delay(20);

  Serial.println(F("ADXL345 pronto."));
  if (COMPENSA_GRAVITA) calibra();

  Serial.print(F("Soglia impostata: "));
  Serial.print(SOGLIA_G, 2);
  Serial.println(F(" g"));
}

void loop() {
  static unsigned long ultimaLettura = 0;
  if (millis() - ultimaLettura < PERIODO_LETTURA_MS) return;
  ultimaLettura = millis();

  float x, y, z;
  if (!leggiAccelerazione(x, y, z)) {
    Serial.println(F("Errore di lettura I2C"));
    return;
  }

  if (COMPENSA_GRAVITA) {
    x -= offsetX;
    y -= offsetY;
    z -= offsetZ;
  }

  bool superaX = fabs(x) > SOGLIA_G;
  bool superaY = fabs(y) > SOGLIA_G;
  bool superaZ = fabs(z) > SOGLIA_G;

  if ((superaX || superaY || superaZ) &&
      millis() - ultimoAvviso >= PAUSA_AVVISI_MS) {
    ultimoAvviso = millis();

    Serial.print(F("!!! AVVISO: soglia superata su asse "));
    if (superaX) Serial.print(F("X "));
    if (superaY) Serial.print(F("Y "));
    if (superaZ) Serial.print(F("Z "));
    Serial.print(F("| X="));  Serial.print(x, 2);
    Serial.print(F(" Y="));   Serial.print(y, 2);
    Serial.print(F(" Z="));   Serial.print(z, 2);
    Serial.println(F(" g"));
  }
}
