/*MODALITà BOMBA
i difensori devono difendere la bomba piazzata in un sito fino allo scadere del tempo,
 gli attaccanti devono recuperarla e disarmarla*/


void bomba() {
  int tmp = inputTimer();
  const unsigned long GAME_DURATION = 60000 * tmp;
  bool startCountdown = false;
  unsigned long COUNTDOWN = 50000;  //valore in ms per il countdown pre-partita
  unsigned long gameStartTime = 0;
  unsigned long pressStart = 0;
  unsigned long val = 0;   // ms di pressione in tempo reale
  int defuseTime = 20000;  //tempo in ms per il disinnesco
  KeyState state = IDLE;
  preGame(5000);
  gameStartTime = millis();  //aggiorna il momento di partenza del countdown con quello della partita effettiva

  //loop che si occupa effettivamente della giocata
  while (true) {
    bool pressed = (digitalRead(BTN_PIN[1]) == LOW);
    unsigned long elapsed = millis() - gameStartTime;
    unsigned long remaining = (elapsed < GAME_DURATION)
                                ? GAME_DURATION - elapsed
                                : 0;
    aggiornaCountdown(remaining);
    if (elapsed < GAME_DURATION && digitalRead(BTN_PIN[1]) == LOW) {
      switch (state) {

        case IDLE:
          if (pressed) {
            pressStart = millis();
            val = 0;
            state = PRESSED;
            Serial.println("⏬ Pulsante premuto...");
          }
          break;

        case PRESSED:
          // Aggiorna e stampa "val" ogni ciclo di loop
          val = millis() - pressStart;
          Serial.println(val);
          ticker(remaining);
          updateProgressBar(val, defuseTime);
          if (val > defuseTime) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("BOMBA");
            lcd.setCursor(0, 1);
            lcd.print("DISINNESCATA");
            timerDone = true;
            timerRunning = false;
            tone(BUZZER_PIN, 1000, 2000);
            delay(10000);
            menu();
            return;
            break;
          }
      }
      if (!pressed) {
        state = IDLE;
        ticker(remaining);
        updateProgressBar(0, defuseTime);
      }
    } else {
      ticker(remaining);
    }


    //questo if si occupa di far terminare la partita se scade il tempo
    if (elapsed > GAME_DURATION) {
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BOMBA");
      lcd.setCursor(0, 1);
      lcd.print("ESPLOSA");
      delay(10000);
      menu();
      return;
    }
  }
}
