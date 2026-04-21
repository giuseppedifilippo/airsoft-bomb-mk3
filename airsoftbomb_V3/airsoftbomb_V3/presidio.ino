/*MODALITà PRESIDIO
due o tre squadre si contendono un sito 
la squadra che raggiunge il sito deve premere il pulsante corrispondente alla 
propria squadra e tenere il sito fino allo scadere del tempo, la squadra che
ha accumulato il maggior tempo sull OBJ vince
respawn illimitati con cooldown raccomandati */



void presidio() {
  lcd.clear();
  int tmp = inputTimer();
  const unsigned long GAME_DURATION = 60000 * tmp;
  lcd.setCursor(0, 0);
  int currentOwner = -1;                         // Chi ha il possesso (-1 = nessuno)
  unsigned long possessTime[3] = { 0, 0, 0 };    // Tempo possesso accumulato per ogni giocatore
  unsigned long ownerStartTime = 0;              // Quando ha preso il possesso l'owner corrente
  unsigned long gameStartTime = 0;               // Quando è iniziata la partita
  unsigned long lastPrintTime = 0;               // Per il countdown seriale
  bool lastBtnState[3] = { false, false, false };
  // ---- STATO GIOCO ----
  enum GameState { WAITING,
                   RUNNING,
                   FINISHED,
                   ARMED };
  GameState gameState = WAITING;
  preGame(5000);
  while (true) {
    bool pressed[3];
    for (int i = 0; i < 3; i++) {
      pressed[i] = (digitalRead(BTN_PIN[i]) == LOW);
    }
    unsigned long elapsed = millis() - gameStartTime;
    unsigned long remaining = (elapsed < GAME_DURATION)
                                ? GAME_DURATION - elapsed
                                : 0;

    switch (gameState) {

      // ---- ATTESA PRIMO PULSANTE ----
      case WAITING:
        for (int i = 0; i < 3; i++) {
          if (pressed[i] && !lastBtnState[i]) {
            // Primo pulsante premuto: avvia partita
            gameState = RUNNING;
            gameStartTime = millis();
            lastPrintTime = millis();
            currentOwner = i;
            ownerStartTime = millis();

            Serial.println("====================================");
            Serial.println("        PARTITA INIZIATA!");
            Serial.println("====================================");
            Serial.print("Possesso iniziale: ");
            Serial.println(BTN_NAME[i]);
            Serial.println();
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print(BTN_NAME[i]);
            tone(BUZZER_PIN, 500, 1000);
          }
        }
        break;

      // ---- PARTITA IN CORSO ----
      case RUNNING:
        {
          unsigned long now = millis();
          unsigned long elapsed = now - gameStartTime;

          // Controlla se la partita è finita
          if (elapsed >= GAME_DURATION) {
            // Accredita l'ultimo possesso
            if (currentOwner >= 0) {
              possessTime[currentOwner] += now - ownerStartTime;
            }
            int winner = 0;
            for (int i = 0; i < 3; i++) {
              unsigned long t = possessTime[i];
              unsigned long ms = t % 1000;
              if (possessTime[i] > possessTime[winner]) winner = i;
            }
            lcd.clear();
            tone(BUZZER_PIN, 2000, 2000);
            lcd.setCursor(0,0);
            lcd.print("VINCE");
            lcd.setCursor(0,1);
            lcd.print(BTN_NAME[winner]);
            gameState = FINISHED;
            delay(10000);
            menu();
            return;
          }

          // Controlla nuove pressioni
          for (int i = 0; i < 3; i++) {
            if (pressed[i] && !lastBtnState[i] && i != currentOwner) {
              // Cambio possesso!
              if (currentOwner >= 0) {
                possessTime[currentOwner] += now - ownerStartTime;
              }
              currentOwner = i;
              ownerStartTime = now;

              Serial.print("[");
              //printTimeLeft(now);
              Serial.print("] Possesso -> ");
              Serial.println(BTN_NAME[i]);
              lcd.clear();
              lcd.setCursor(0, 1);
              lcd.print(BTN_NAME[i]);

              
            }
          }

          // Stampa countdown ogni secondo
          if (now - lastPrintTime >= 1000) {
            lastPrintTime = now;
            Serial.print("  |  In possesso: ");
            Serial.println(currentOwner >= 0 ? BTN_NAME[currentOwner] : "Nessuno");
            aggiornaCountdown(remaining);
          }

          break;
        }

      // ---- PARTITA FINITA ----
      case FINISHED:
        // Non fa nulla finché non si preme reset
        // (opzionale: aggiungi un pulsante reset o aspetta un secondo e riparti)
        delay(5000);
        //resetGame();
        break;
    }

    // Salva stato precedente pulsanti
    for (int i = 0; i < 3; i++) {
      lastBtnState[i] = pressed[i];
    }
  }

  //SEQUENZA DI CHIUSURA
  lcd.clear();
  lcd.print("premi A per restart");
  while (true) {
    char customKey = customKeypad.getKey();
    if (customKey == 'A') {
      return;
    }
  }
}
