/*CONTO ALLA ROVESCIA WIP
semplice conto alla rovescia per normali partite
allerta con la sirena partenza e scadenza parita
*/

void timer() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TEMPO");
  lcd.setCursor(0, 1);
  lcd.print("PARTITA");
  delay(1000);
  int tmp = inputTimer();
  unsigned long GAME_DURATION = 60000 * tmp;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TEMPO");
  lcd.setCursor(0, 1);
  lcd.print("LUCE A LUCE VERDE");
  tmp = inputTimer();
  unsigned long green_light = tmp * 60000;
  preGame(green_light);
  while (true) {
    unsigned long elapsed = millis() - gameStartTime;
    unsigned long remaining = (elapsed < GAME_DURATION)
                                ? GAME_DURATION - elapsed
                                : 0;
    aggiornaCountdown(remaining);
    if (digitalRead(BTN_PIN[1]) == LOW) {
      //PARTITA TERMINATA IN ANTICIPO
    }
    if (elapsed > GAME_DURATION) {
      //PARTITA TERMINATA
      tone(BUZZER_PIN, 1500, 2000 );
      delay(5000);
      menu();
      return;
      
    }
  }
}