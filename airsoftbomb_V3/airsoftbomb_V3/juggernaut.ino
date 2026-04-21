/*MODALITà JUGGERNAUT WIP
pensata per la milsim OPERAZIONE BRUNEVOY
richiede la ricerca di una tessera/codice di attivazione per prendere controllo o disattivare
il juggernaut, se cambio qualche parole può diventare un comune obiettivo per giocate*/

void juggernaut() {
  //sta prima da capire come mettere il lettore di tessere e il modulo radio
  KeyState state = IDLE;
  char* codice = "6769";
  unsigned long pressStart = 0;
  unsigned long val = 0;  // ms di pressione in tempo reale
  unsigned long deactTime = 180000;
  preGame(1000);
  juggMenu();
  while (true) {
    char customKey = customKeypad.getKey();
    //modalità disarmo
    if (customKey == '1') {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("disarmo");
      while (true) {
        bool pressed = (digitalRead(BTN_PIN[1]) == LOW);
        char customKey = customKeypad.getKey();
        if (customKey == 'B') {
          juggMenu();
          break;
        }
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
            //   ticker(remaining);
            updateProgressBar(val, deactTime);
            if (val > deactTime) {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("JUGGERNAUT");
              lcd.setCursor(0, 1);
              lcd.print("DISATTIVATO");
              timerDone = true;
              timerRunning = false;
              tone(BUZZER_PIN, 1000, 2000);
              delay(4000);
              return;
              break;
            }
        }

        if (!pressed) {
          state = IDLE;
          updateProgressBar(0, deactTime);
          //ticker(remaining);
        }
      }
    } else if (customKey == '2') {  //modalità controllo
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("inserire codice");
      while (true) {
        bool pressed = (digitalRead(BTN_PIN[1]) == LOW);
        char customKey = customKeypad.getKey();
        if (customKey == 'B') {
          juggMenu();
          break;
        }
      }
    }
  }
}

void juggMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DISATTIVA (1)");
  lcd.setCursor(0, 1);
  lcd.print("CONTROLLO (2)");
  return;
}