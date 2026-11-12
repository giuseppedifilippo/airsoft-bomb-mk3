/*implementazione per varie impostazioni di gioco WIP
richiedebbe l implementazione di eeprom.h per poter salvare i valori anche dopo lo spegnimento
lista di possibili valori da salvare in mameria
-tempo per il defuse
-beep si/no della bomba quando attiva
-sequenza di disinnesco con codice/pressione pulsante
-finestra allerta buzzer

IN DISUSO PER LAVORO FUTURO
*/

char* settings[] = { "tempo disarmo", "beep bomba", "tipo disinnesco", "tempo buzzer" };
char* disinnesco[] = { "codice", "pressione", "off" };
void impostazioni() {
  int setIndex = 0;
  pulisciescrivi("<*    SELEZIONA   #>", 0, 0);
  scrivi(settings[setIndex], 0,1);
  while (true) {
    char customKey = customKeypad.getKey();
    if (customKey == 'B') {  //salva le eventuali modifiche e torna al menù iniziale
      updateSettings();
      menu();
      return;
    } else if (customKey == '#' && setIndex < 4) {
      index += 1;
      beep();
      lcd.setCursor(0, 1);
      lcd.print(settings[index]);
    } else if (customKey == '*' && setIndex >= 1) {
      beep();
      index -= 1;
      lcd.setCursor(0, 1);
      lcd.print(settings[setIndex]);
    } else if (customKey == 'A') {
      switch (setIndex) {
      }
    }
  }
}
//aggiorna i valori salvati sulla eeprom
void updateSettings() {
  EEPROM.update(0, defuseTime);
}
