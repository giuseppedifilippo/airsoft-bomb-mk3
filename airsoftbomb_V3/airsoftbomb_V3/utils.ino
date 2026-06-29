/*PAGINA DEDICATA A VARIE FUNZIONI UTILITARIE*/


//si occupa del beep del buzzer con ritmo incrementale fino allo scadere

void ticker(unsigned long remaining) {


  // ── Buzzer graduale (ultimi 30 secondi) ───────────────────
  {
    const unsigned long WARN_WINDOW_MS = 30000UL;  // ultimi 30 s
    unsigned long now = millis();
    // Frequenza:  400 Hz  → 2000 Hz
    const unsigned int FREQ_MIN = 400;
    const unsigned int FREQ_MAX = 2000;

    // Intervallo tra beep: 1000 ms → 80 ms
    const unsigned long INT_MIN = 40UL;
    const unsigned long INT_MAX = 1000UL;

    // Durata del tono: 150 ms → 40 ms
    const unsigned long TONE_MIN = 40UL;
    const unsigned long TONE_MAX = 150UL;

    static unsigned long lastBeep = 0;
    static bool toneActive = false;
    static unsigned long toneStarted = 0;
    static unsigned long curInterval = INT_MAX;
    static unsigned long curToneDur = TONE_MAX;
    static unsigned int curFreq = FREQ_MIN;

    if (remaining > WARN_WINDOW_MS) {
      // Fuori dalla finestra: nessun beep
      if (toneActive) {
        noTone(BUZZER_PIN);
        toneActive = false;
      }
      return;
    }

    // progress: 0.0 = inizio finestra (30 s rimasti)
    //           1.0 = timer scaduto   (0 s rimasti)
    float progress = 1.0f - (float)remaining / (float)WARN_WINDOW_MS;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    // Interpolazione lineare dei parametri
    curFreq = (unsigned int)(FREQ_MIN + progress * (FREQ_MAX - FREQ_MIN));
    curInterval = (unsigned long)(INT_MAX - progress * (INT_MAX - INT_MIN));
    curToneDur = (unsigned long)(TONE_MAX - progress * (TONE_MAX - TONE_MIN));

    // ── Macchina a stati: ON / OFF del singolo beep ─────────
    if (toneActive) {
      // Controlla se il tono deve finire
      if (now - toneStarted >= curToneDur) {
        noTone(BUZZER_PIN);
        toneActive = false;
        lastBeep = now;
      }
    } else {
      // Controlla se è il momento di un nuovo beep
      if (now - lastBeep >= curInterval) {
        tone(BUZZER_PIN, curFreq);
        toneActive = true;
        toneStarted = now;
      }
    }
  }
}

void initProgressBarChars() {
    // 5 blocchi parziali (da 1/5 a 5/5 di cella piena)
    byte partial1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00}; // 1/5
    byte partial2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}; // 2/5
    byte partial3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x00}; // 3/5
    byte partial4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x00}; // 4/5
    byte partial5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x00}; // 5/5 (pieno)

    lcd.createChar(0, partial1);
    lcd.createChar(1, partial2);
    lcd.createChar(2, partial3);
    lcd.createChar(3, partial4);
    lcd.createChar(4, partial5);
}

//Funzione per aggiornare la barra di progresso
void updateProgressBar(unsigned long val, unsigned long total) {
    // Riga 0: percentuale
    /*unsigned int percent = (unsigned int)((val * 100UL) / total);
    lcd.setCursor(0, 0);
    lcd.print("Progress: ");
    if (percent < 10)  lcd.print("  ");
    else if (percent < 100) lcd.print(" ");
    lcd.print(percent);
    lcd.print("%");*/

    // Riga 1: barra grafica
    // 16 colonne x 5 pixel-colonne = 80 "pixel" orizzontali totali
    const uint8_t BAR_COLS = 16;
    unsigned long pixels = (val * (unsigned long)(BAR_COLS * 5)) / total;

    uint8_t fullBlocks  = pixels / 5;   // celle completamente piene
    uint8_t remainder   = pixels % 5;   // pixel parziali nell'ultima cella

    lcd.setCursor(0, 1);
    for (uint8_t i = 0; i < BAR_COLS; i++) {
        if (i < fullBlocks) {
            lcd.write(byte(4));      // blocco pieno
        } else if (i == fullBlocks && remainder > 0) {
            lcd.write(byte(remainder - 1)); // blocco parziale (char 0..3)
        } else {
            lcd.print(" ");          // spazio vuoto
        }
    }
}

//Funzione per aggionare il timer a schermo, prende come argomento il tempo passato dall inizio timer da gestire dal chiamante
void aggiornaCountdown(unsigned long remaining) {
  static char lastLine[17] = "";

  unsigned int mm = remaining / 60000UL;
  unsigned int ss = (remaining % 60000UL) / 1000UL;

  char line[17];
  snprintf(line, sizeof(line), "  %02u:%02u     ", mm, ss);

  if (memcmp(line, lastLine, 16) != 0) {
    memcpy(lastLine, line, 16);
    lcd.setCursor(0, 0);
    lcd.print(line);
  }
}

//sequenza di input per timer generico, ritorna il valore per il timer come int non chiama un funzione timer
int inputTimer() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("imposta timer");
  lcd.setCursor(2, 1);
  lcd.print("mm conferma(A)");
  lcd.setCursor(0, 1);
  lcd.cursor();
  while (true) {
    char customKey = customKeypad.getKey();
    if (customKey == 'A' && inputBuffer != "") {  //avvia il timer con il valore indicato se premuto il pulsante A
      minuti = inputBuffer.toInt();
      beep();
      inputBuffer = "";
      lcd.clear();
      return minuti;
    }
    //IGNORA L INPUT SE è BCD
    if (forbidden.indexOf(customKey) != -1) {
      Serial.println(customKey);
      continue;
    }

    Serial.println(inputBuffer);
    if (customKey) {
      if (check) {
        beep();
        lcd.print(customKey);
        lcd.setCursor(0, 1);
        check = false;
        inputBuffer += String(customKey);
        Serial.println(customKey);
      } else {
        beep();
        inputBuffer += String(customKey);
        lcd.print("  ");
        lcd.setCursor(0, 1);
        lcd.print(customKey);
        lcd.setCursor(1, 1);
        check = true;
        Serial.println(customKey);
      }
    }
    if (inputBuffer.length() >= 3) {
      inputBuffer = String(inputBuffer.charAt(inputBuffer.length() - 1));  // il subslicing non funziona come dovrebbe con questo input, NON TOCCARE
    }
  }
}


//funzione che crea un countdown universale prima di avviare la partita effettiva
void preGame(unsigned long  COUNTDOWN) {
  lcd.setCursor(0, 0);
  lcd.print("PREMI B");
  lcd.setCursor(0, 1);
  lcd.print("PER AVVIARE");
  bool startCountdown = false;
  unsigned long startTime = 0;

  while (true) {
    unsigned long elapsed = millis() - startTime;
    unsigned long remaining = (elapsed < COUNTDOWN)
                                ? COUNTDOWN - elapsed
                                : 0;
    char customKey = customKeypad.getKey();
    if (customKey == 'B') {
      lcd.clear();
      beep();
      startCountdown = true;
      startTime = millis();
      continue;
    }
    if (startCountdown) {
      aggiornaCountdown(remaining);
      if (elapsed > COUNTDOWN) {
        tone(BUZZER_PIN, 800, 1000);
        return;
      }
    }
  }
}


//usata per far fare beep alla pressione dei pulsanti senza che sto scrivere ogni volta tone()
void beep() {
  tone(BUZZER_PIN, 1500, 100);
  return;
}


//funzione da invocare prima di chiudere la funzione di una modalità e tornare al menù di partenza
void menu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("<* SELEZIONA  #>");
  lcd.setCursor(0, 1);
  lcd.print(modes[index]);
  return;
}
