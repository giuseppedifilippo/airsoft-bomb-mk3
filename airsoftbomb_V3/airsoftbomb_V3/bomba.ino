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
    if (elapsed < GAME_DURATION && pressed) {
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
          ticker(remaining);
          updateProgressBar(val, defuseTime);
          if (val > defuseTime) {
            printStatoBomba("DISINNESCATA");
            timerDone = true;
            timerRunning = false;
            tone(BUZZER_PIN, 1000, 2000);
            delay(10000);
            menu();
            return;
            break;
          }
      }
      /*if (!pressed) {
        val = 0;
        
        ticker(remaining);
      }*/
    } else if (!pressed) {
      val = 0;
      updateProgressBar(0, defuseTime);
      state = IDLE;
      ticker(remaining);
    }


    //questo if si occupa di far terminare la partita se scade il tempo
    if (elapsed > GAME_DURATION) {
      delay(1000);
      printStatoBomba("ESPLOSA");
      delay(10000);
      menu();
      return;
    }
  }
}