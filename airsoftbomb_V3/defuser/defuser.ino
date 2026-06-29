#include <FastLED.h>
#include <LiquidCrystal_I2C.h>
#define POT1 4
#define POT2 5
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int offset = 100;
long target, parziale1, parziale2;
long percentuale;

void initBarChars() {
  byte p1[8] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 };  // 1/5
  byte p2[8] = { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 };  // 2/5
  byte p3[8] = { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x00 };  // 3/5
  byte p4[8] = { 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x00 };  // 4/5
  byte p5[8] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00 };  // 5/5 (pieno)

  lcd.createChar(0, p1);
  lcd.createChar(1, p2);
  lcd.createChar(2, p3);
  lcd.createChar(3, p4);
  lcd.createChar(4, p5);
}
void mostraBarraPot(int valorePot, uint8_t riga) {
    const uint8_t  BAR_COLS  = 16;             // larghezza barra in celle
    const uint16_t TOT_PIXEL = BAR_COLS * 5;   // 80 pixel orizzontali totali
    const unsigned long INTERVALLO = 50;       // ms tra due aggiornamenti

    // Stato persistente tra una chiamata e l'altra
    static unsigned long ultimoAggiornamento = 0;
    static int16_t ultimoPixel = -1;           // -1 = "mai disegnata"

    // 1) Throttling temporale: non fare nulla se non e' passato abbastanza tempo
    unsigned long ora = millis();
    if (ora - ultimoAggiornamento < INTERVALLO) {
        return;
    }
    ultimoAggiornamento = ora;

    // 2) Vincola il valore al range valido (sicurezza)
    if (valorePot < 0)    valorePot = 0;
    if (valorePot > 1023) valorePot = 1023;

    // 3) Scala 0-1023 -> 0-80 pixel
    uint16_t pixel = ((uint32_t)valorePot * TOT_PIXEL) / 1023;

    // 4) Ridisegna solo se la barra e' effettivamente cambiata (anti-flicker)
    if ((int16_t)pixel == ultimoPixel) {
        return;
    }
    ultimoPixel = (int16_t)pixel;

    uint8_t celleP = pixel / 5;   // celle completamente piene
    uint8_t resto  = pixel % 5;   // pixel parziali nell'ultima cella

    lcd.setCursor(0, riga);
    for (uint8_t i = 0; i < BAR_COLS; i++) {
        if (i < celleP) {
            lcd.write(byte(4));            // cella piena
        } else if (i == celleP && resto > 0) {
            lcd.write(byte(resto - 1));    // cella parziale (char 0..3)
        } else {
            lcd.print(" ");                // cella vuota
        }
    }
}

void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  initBarChars();
  pinMode(POT1, INPUT);
  pinMode(POT2, INPUT);
  Serial.begin(9600);
  Serial.print("ready");
  randomSeed(analogRead(POT1) + analogRead(POT2));
  target = random(1024);
  parziale1 = random(target);
  parziale2 = target - parziale1;
}

void loop() {
  int val1 = analogRead(POT1);
  int val2 = analogRead(POT2);
  mostraBarraPot(val1, 1);
  mostraBarraPot(val2, 2);
  //inndica la distanza dei valori dal target
  percentuale = ((target - abs(target - (val1 + val2))) / target) * 100;

  Serial.println(val1);
  Serial.println(val2);
  //quando il valore viene azzeccato manda il messaggio di ok
  //operazione da rifare: una volta creato il valore deve essere diviso in due numeri la cui somma faccia il target
  if (val1 + offset < parziale1 && val1 - offset > parziale1) {
    Serial.print("ok");
  }
}
