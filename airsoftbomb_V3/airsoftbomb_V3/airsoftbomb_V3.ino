/*Main per questo progetto in questo file sono solo contenute le variabili globali e il menù di selezione modalità
DISPOTIVO CHE FUNGE DA VARI TIPI DI OBIETTIVO PER VARIE GIOCATE SOFTAIR
*/

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
//inizializza istanza lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define BUZZER_PIN 24
//PARAMETRI PER IL TASTIERINO ALFANUMERICO
const byte ROWS = 4;  //four rows
const byte COLS = 4;  //four columns
//define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  { 'A', '3', '2', '1' },
  { 'B', '4', '5', '6' },
  { 'C', '7', '8', '9' },
  { 'D', '#', '0', '*' }
};

//array conenenti i pin per le righe e colonne dei tasti del tastierino 
byte rowPins[ROWS] = { 52, 50, 48, 46 };  
byte colPins[COLS] = { 38, 40, 42, 44 };  
//funzione che inizializza l'oggetto del tastierino
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


//VARIABILI UNIVERSALI PER IL TIMER (valori in millisecondi)
unsigned long timerStart = 0;
unsigned long timerDuration = 0;  
unsigned long lastPrint = 0;
bool timerRunning = false;
bool timerDone = false;
int minuti = 0;

//Variabili per il sistema di input
bool check = false;
String inputBuffer = "";
String forbidden = "*#BCD";  //caratteri da ignorare per certe sequenze di input

// ---- PIN CONFIGURATION ----
const int BTN_PIN[3] = { 8, 9, 10 };
const String BTN_NAME[3] = { "AMERICA", "CINA", "RUSSIA" };

//ARRAY DELLE VARIE MODALITà
char* modes[] = { "      BOMBA    ",
                  "    PRESIDIO   ",
                  "  JUGGERNAUT   ",
                  "CERCA&DISTRUGGI",
                  " IMPOSTAZIONI  ",
                  "  COUNTDOWN    "};


int index = 0;
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    pinMode(BTN_PIN[i], INPUT_PULLUP);  // Pullup interno: LOW = premuto
  }
  lcd.init();
  lcd.noCursor();
  lcd.backlight();
  initProgressBarChars();
  lcd.clear();
  delay(50);
  lcd.print("Difilippo");
  lcd.setCursor(0, 1);  //param1 = X   param2 = Y
  lcd.print("Industries");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("<* SELEZIONA  #>");
  lcd.setCursor(0, 1);
  lcd.print(modes[index]);
  Serial.begin(115200);
  while (true) {
    char customKey = customKeypad.getKey();
    if (customKey == '#' && index < 4) {
      index += 1;
      beep();
      lcd.setCursor(0, 1);
      lcd.print(modes[index]);
    } else if (customKey == '*' && index >= 1) {
      beep();
      index -= 1;
      lcd.setCursor(0, 1);
      lcd.print(modes[index]);
    } else if (customKey == 'A') {
      beep();
      switch (index) {
        case 0:
          bomba();
          break;
        case 1:
          presidio();
          break;
        case 2:
          juggernaut();
          break;
        case 3:
          cerca_e_distruggi();
          break;
        case 4 :
        impostazioni();
        break;
      }
    }
  }
}

void loop() {
  //lasciare vuoto, il loop viene eseguito nelle varie funzioni che si occupano di gestire le singole modalità
}






















/*                                                                          
                                                                                
                                                                                
                      .....................................                     
                    ..-+++++++++++++++++++++++++++++++++++-..                   
                   ..=#######*-+*+==#*+*###################=..                  
                  .:+########:+%%%#::+#*:*+*################+:.                 
                ..-*########+.=%%#=:.=*=.=#+=**##############*-..               
               ..=#*########*.::::::::::.-=-.*+=###############=..              
              .:+############::::::.::::.::::=-:+=##############+..             
            ..-*#############-:::::::::::::::::-*:+##############*:..           
           ..=###############:::::::::::::::::::::+################=..          
           ..=*##############:::::::::::::::::::::+################+..          
            ..-*####*########=::::::::::::::::::::=###############=..           
              .:+#############-:::::::::::::::::::=#############*-..            
               ..=*############+:::::::::::::::::.*############+:.              
                ..:*############*::::::::::::::::-############=..               
                  ..+############=::::::::::::::.*##########*-..                
            .:..   ..-###########+.:::::::::::::-##########+:.     ..           
            .:*+==-:..:*#########+:::::::::::::.+#########=.....:-=..           
             .=**+=--...+########=:::::::::::::.########*-..:-=+**-.            
             .:*+...:-...-#######=:::::::::::::-#######*:..:-:..=+..            
         .:-=+**::+==--:..:===++*+.:::::::::::.***+++==....:-==-.**+-:.         
          .-=+*-.++:-=++**++==----:::::::::::::::---===++++*=-=*:-**+=:.        
              ..=*--=.:-=.=+:+=-*==+++*++++**+++*+=====--:==:--+=.:.            
              .:*=.=:-.=.:--.-.::-=:-.--:-:=*---:-.-.:.:-.=:---:*-.             
              .-+*++*==-=:-.-.:-.+--:-.-.+:=*--=.::.-.-.=----+=+*=.             
                 .:--=++***+*+=+=--*-+-=:--**=--+-*++=*+**++==-:.               
                       ...::----====+++++++++++====---::...                     
                                 ...-=-==--==:..                                
                                   .:+######=..                                 
                                    ..=###*-..                                  
                                     ..-*+:.                                    
                                       .:..                                     
                                        ..                                      
                                                                                
 */
