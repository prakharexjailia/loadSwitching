#include <Keypad.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// with the arduino pin number it is connected to
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'#','0','*'},
  {'9','8','7'},
  {'6','5','4'},
  {'3','2','1'}
};
byte rowPins[ROWS] = {A0,A1,A2,A3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {10,9,8}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int time[] = {0,0,0,0,0,0,0,0,0,0,0,0}; // in case of array &time and time will be equal

void setup(){
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  //EEPROM.begin(512);
  lcd.print("reading EEPROM");
  readEEPROM(time);
  delay(2000);
  lcd.clear();
  lcd.print("Start enter time");
  delay(2000);
  lcd.clear();
  //display time
  displayTime();
  //get time input
  bool x = getTimeInput(time);
  if(x==true){
    lcd.clear();
    lcd.print("Wrt to EEPROM");
    writetoEEPROM(time);
    delay(2000);
  }
  //display time
  displayTime();
  
}

void loop(){

}


//reading data from EEPROM
void readEEPROM(int *time){
  int addr = 0;
  for(int i=0; i<=11; i++){ //this loop for inserting data
    *time = (int)EEPROM.read(addr); //getting data from array
    addr = addr + 1;
    // advance to the next address.  there are 512 bytes in but in this case we insert 11 byte
    // save all changes to the flash.
    time += 1;
  }
}


//writting data to EEPROM
void writetoEEPROM(int *time){
  int addr = 0;
  //get value
  for(int i=0; i<=11; i++){ //this loop for getting data
  
    EEPROM.write(addr, *time);  //writting data to EEPROM
    addr = addr + 1;
    // advance to the next address.  there are 512 bytes in but in this case we insert 11 byte
    // save all changes to the flash.
    time += 1;
  }
  //EEPROM.commit();
}


void displayTime(){
  lcd.noBlink();
  lcd.clear();
  lcd.setCursor(0,0); //setCursor(coloum,row);
  lcd.print(time[0]);
  lcd.print(time[1]);
  lcd.print(":");
  lcd.print(time[2]);
  lcd.print(time[3]);
  lcd.setCursor(0,1);
  lcd.print(time[4]);
  lcd.print(time[5]);
  lcd.print("/");
  lcd.print(time[6]);
  lcd.print(time[7]);
  lcd.print("/");
  lcd.print(time[8]);
  lcd.print(time[9]);
  lcd.print(time[10]);
  lcd.print(time[11]);
  lcd.setCursor(0,0);
}


bool getTimeInput(int *time){
  // time is containing addr of first value of array
  // *time is equal to first value of array
  // to get second value of array we need to add 1 int time to get next addr to point next value
  int count = 0; //using for printing allignment
  bool active = true; //for loop iteration and wait for "#" to be pressed like enter or confirmation.
  while(active){
    char key = keypad.getKey();
    lcd.blink();
    if(35 == int(key)){   //all Set # askii value
      active = false;
      break;
    } 
    if(42 == int(key)){   //fill input from start
      count = 0;
      lcd.clear();
      displayTime();
    }
    if (key>47) {
      if(count==0){
        lcd.setCursor(0,0);
      }else if(count==2){
        lcd.blink();
        lcd.setCursor(3,0);
      }else if(count==4){
        lcd.blink();
        lcd.setCursor(0,1);
      }else if(count==6){
        lcd.blink();
        lcd.setCursor(3,1);
      }else if(count==8){
        lcd.blink();
        lcd.setCursor(6,1);
      }
      *time = key-48;     //assign key value to array askii replacement
      lcd.print(*time); //printing value entered on screen
      time += 1;    //incrementing addr of time to get next addr of array
      count++;      
    }  
  }
  delay(1000);
  lcd.clear();
  lcd.print("All set");
  delay(1000);
  lcd.clear();
  return true;
}
