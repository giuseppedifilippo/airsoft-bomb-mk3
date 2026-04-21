/*MODALITà CERCA E DISTRUGGI
il team di attacco porta con se la bomba che dovono piazzare e armarla in uno dei due siti disponibili
entro la scadere del tempo, la squadra di difesa deve impedire l' armamento, 
nel caso che la bomba venga piazzata la squadra di difesa deve disarmare la bomba entro la scadere 
del timer di detonazione, sarebbe meglio se implemento la radio altrimenti non ha senso la giocata*/

void cerca_e_distruggi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("tempo");
  lcd.setCursor(0, 1);
  lcd.print("partita");
  delay(1000);
  KeyState state = IDLE;
  unsigned long pressStart = 0;
  int defuseTime = 20000;            //tempo in ms per il disinnesco
  unsigned long val = 0;             // ms di pressione in tempo reale
  int tmp = inputTimer();
  unsigned long GAME_DURATION = 60000 * tmp;  //tempo totale della partita
  Serial.println(GAME_DURATION);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("timer");
  lcd.setCursor(0, 1);
  lcd.print("detonazione");
  delay(1000);
  int minuti_det = inputTimer();  // tempo del detonatore
  //finta sequena di armamento, basta premere 7 tasti a caso e la bomba si arma, giusto per fare una cosa stile CS
  const char allowed[] = "ABCD0123456789*#";
  int count = 0;
  lcd.setCursor(0, 1);

  // questo loop si uccupa countdown della partita se viene attivata la bomba, il loop passa a quello della detonazione
  preGame(5000);
  unsigned long gameStartTime = millis();
  while (true) {
    unsigned long elapsed = millis() - gameStartTime;
    unsigned long remaining = (elapsed < GAME_DURATION)
                                ? GAME_DURATION - elapsed
                                : 0;
    aggiornaCountdown(remaining);
    if (count == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ARMAMENTO");
      lcd.setCursor(count, 1);
    }
    char customKey = customKeypad.getKey();
    if (customKey == '1') {
      Serial.println(customKey);
      lcd.setCursor(count, 1);
      lcd.print(customKey);
      tone(BUZZER_PIN, 800, 100);
      count += 1;
    }
    if (count > 7) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BOMBA");
      lcd.setCursor(0, 1);
      lcd.print("ARMATA");
      gameStartTime = millis();
      GAME_DURATION = 60000 * minuti_det;
      break;  // attivare il conto alla rovescia
    }
    if (elapsed > GAME_DURATION) {
      // BOMBA NON PIANTATA PARTITA TERMINATA VINCE SQUADRA DIFESA
      Serial.println("qui");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("VINCE");
      lcd.setCursor(0, 1);
      lcd.print("DIFESA");
      delay(10000);
      return;
    }
  }

  //questo loop viene attivato solo se la bomba viene piantata
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
          if (val > defuseTime) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("BOMBA");
            lcd.setCursor(0, 1);
            lcd.print("DISINNESCATA");
            timerDone = true;
            timerRunning = false;
            tone(BUZZER_PIN, 1000, 2000);
            delay(4000);
            return;
          }
         

          if (!pressed) {
            state = IDLE; 
            updateProgressBar(0, defuseTime);
            ticker(remaining);
          }
      }
    } else {
      ticker(remaining);
    }


    //LA BOMBA è ESPLOSA SQUADRA ATTACCO VINCE
    if (elapsed > GAME_DURATION) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("VINCE");
      lcd.setCursor(0, 1);
      lcd.print("ATTACCO");
      delay(10000);
      return;
    }
  }
}
