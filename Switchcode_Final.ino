#include <Keypad.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;        //declering class variable for rtc

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

int time0[] = {0,0,0,0,0,0};     // in case of array &time and time will be equal
int time1[] = {0,0,0,0,0,0}; 
int time2[] = {0,0,0,0,0,0};

//no of loads
int loads = 2;

//for timecountdown
long int netTime0;
long int netTime1;
long int netTime2;

long int triggerTime0 = 20;
long int triggerTime1 = 20;
long int triggerTime2 = 20;

int onOffTriggerTime0 = 1;
int onOffTriggerTime1 = 1;
int onOffTriggerTime2 = 1;

int state1load = 0;
int state2load = 0;
int preventSwitching = 0;

void setup(){
   pinMode(11, OUTPUT);
   pinMode(12, OUTPUT);
   digitalWrite(11, HIGH);      //LOAD 1   OFF  -ve logic based on relay
   digitalWrite(12, HIGH);       //LOAD 2  OFF  -ve logic based on relay
   // set up the LCD's number of columns and rows:
   lcd.begin(16, 2);
   
   while (! rtc.begin()) {                      //exception handling for RTC in case no RTC
    lcd.print("Didn't find RTC");
    delay(500);
    lcd.clear();
    delay(500);
  }
  
  //EEPROM.begin(512);
  lcd.print("Reading EEPROM");
  readEEPROM(time0,time1,time2);
  delay(2000);
  lcd.clear();
  lcd.print("Start enter time");
  delay(2000);
  lcd.clear();
  //display time
  displayTime(time0);
  //get time input
  bool x = getTimeInput(time0);          //time entered to first time array

  lcd.clear();
  //display time
  displayTime(time1);
  //get time input
  bool y = getTimeInput(time1);          //time entered to second time array

  lcd.clear();
  //display time
  displayTime(time2);
  //get time input
  bool z = getTimeInput(time2);          //time entered to third time array
  
  if(x==true && y==true && z==true){
    lcd.clear();
    lcd.print("Wrt to EEPROM");
    writetoEEPROM(time0,time1,time2);
    delay(2000);
  }
  
  lcd.clear();

  //no of load information as keypad input
  lcd.print("loads in(1||2):"); 
  loads = inputLoads();

  //calculate time
  netTime0 = ((((time0[0]*10)+time0[1])*3600) + (((time0[2]*10)+time0[3])*60) + (((time0[4]*10)+time0[5])));    //(time[0]*10)+time[1]) = h, (time[2]*10)+time[3]) = m 
  netTime1 = ((((time1[0]*10)+time1[1])*3600) + (((time1[2]*10)+time1[3])*60) + (((time1[4]*10)+time1[5])));
  netTime2 = ((((time2[0]*10)+time2[1])*3600) + (((time2[2]*10)+time2[3])*60) + (((time2[4]*10)+time2[5])));
}

void loop(){
   DateTime now = rtc.now();

   if(onOffTriggerTime0){
    triggerTime0 = countDown(netTime0, now.hour(), now.minute(), now.second());                  //input time from RTC h:m:s
  }
  if(onOffTriggerTime1){
    triggerTime1 = countDown(netTime1, now.hour(), now.minute(), now.second());                  //input time from RTC h:m:s
  }
  if(onOffTriggerTime2){
    triggerTime2 = countDown(netTime2, now.hour(), now.minute(), now.second());                  //input time from RTC h:m:s
  }
  
  if(loads==1){
    if(triggerTime0 <= 0 && triggerTime1 > 0){
        turnONLoad1(&state1load);
      }
    if(triggerTime0 <= 0 && triggerTime1 <= 0){
        turnOFFLoad1(&state1load);
      }
  }
  if(loads==2){
    if(triggerTime0 <= 0 && triggerTime1 > 0 && triggerTime2 > 0){
        turnONLoad1(&state1load);
      }
    if(triggerTime0 <= 0 && triggerTime1 <= 0 && triggerTime2 > 0){
      if(preventSwitching == 0){
           switchLoads(&state1load, &state2load);
           preventSwitching++;
        }
        
      }
    if(triggerTime0 <= 0 && triggerTime1 <= 0 && triggerTime2 <= 0){
        turnOFFLoad2(&state2load);
      }
  }

  lcd.clear();
  lcd.print("LOAD:1    LOAD:2");
  lcd.setCursor(0,1);
  lcd.print(state1load);
  lcd.print("         ");
  lcd.print(state2load);
  delay(1000);
}

void turnONLoad1(int *state1load){
    digitalWrite(11, LOW);      //LOAD 1   ON
    *state1load = 1;
  }


void turnOFFLoad1(int *state1load){
    digitalWrite(11, HIGH);      //LOAD 1   OFF
    *state1load = 0;
  }

void turnOFFLoad2(int *state2load){
    digitalWrite(12, HIGH);      //LOAD 2   OFF
    *state2load = 0;
  }

void switchLoads(int *state1load, int *state2load){
    if(*state1load){
        digitalWrite(11, HIGH);      //LOAD 1   OFF
        *state1load = 0;
        digitalWrite(12, LOW);      //LOAD 2   ON
        *state2load = 1;
      }else{
        digitalWrite(11, LOW);      //LOAD 1   ON
        *state1load = 1;
        digitalWrite(12, HIGH);      //LOAD 2   OFF
        *state2load = 0;
      }
  }

//int countDown(future time, currunt time)
int countDown(long int time1, int h2, int m2, int s2){
    long int netTime2;
    netTime2 = h2*3600 + m2*60 + s2;
    return time1 - netTime2;
}

int inputLoads(){
  int loads = 2;
  int active = 1;
  while(active){
    char key = keypad.getKey();
    if (key) {
      lcd.setCursor(15,0);
      loads = key-48;
      lcd.print(loads);
      active = 0;
    }   
  }
  delay(2000);
  lcd.clear();
  return loads;
}


//reading data from EEPROM
void readEEPROM(int *time0,int *time1, int *time2){
  int addr = 0;
  for(int i=0; i<=5; i++){ //this loop for inserting data
    *time0 = (int)EEPROM.read(addr); //getting data from array
    addr = addr + 1;
    // advance to the next address.  there are 512 bytes in but in this case we insert 11 byte
    // save all changes to the flash.
    time0 += 1;
  }
  for(int i=0; i<=5; i++){ //this loop for inserting data
    *time1 = (int)EEPROM.read(addr); //getting data from array
    addr = addr + 1;
    // advance to the next address.  there are 512 bytes in but in this case we insert 11 byte
    // save all changes to the flash.
    time1 += 1;
  }
  for(int i=0; i<=5; i++){ //this loop for inserting data
    *time2 = (int)EEPROM.read(addr); //getting data from array
    addr = addr + 1;
    // advance to the next address.  there are 512 bytes in but in this case we insert 11 byte
    // save all changes to the flash.
    time2 += 1;
  }
}


//writting data to EEPROM
void writetoEEPROM(int *time0, int *time1, int *time2){
  int addr = 0;
  //get value
  for(int i=0; i<=5; i++){                                     //this loop for getting data
    EEPROM.write(addr, *time0);                                 //writting data to EEPROM
    addr = addr + 1;
                                                                // advance to the next address.  there are 512 bytes in but in this case we insert 6 byte
                                                                // save all changes to the flash.
    time0 += 1;
  }
  for(int i=0; i<=5; i++){ //this loop for getting data
  
    EEPROM.write(addr, *time1);  //writting data to EEPROM
    addr = addr + 1;
    // advance to the next address.  there are 512 bytes in but in this case we insert 11 byte
    // save all changes to the flash.
    time1 += 1;
  }
  for(int i=0; i<=5; i++){ //this loop for getting data
  
    EEPROM.write(addr, *time2);  //writting data to EEPROM
    addr = addr + 1;
    // advance to the next address.  there are 512 bytes in but in this case we insert 11 byte
    // save all changes to the flash.
    time2 += 1;
  }
  //EEPROM.commit();
}


void displayTime(int *time){
  lcd.noBlink();
  lcd.clear();
  lcd.setCursor(0,0); //setCursor(coloum,row);
  lcd.print(*time);
  lcd.print(*(time+1));
  lcd.print(":");
  lcd.print(*(time+2));
  lcd.print(*(time+3));
  lcd.print(":");
  lcd.print(*(time+4));
  lcd.print(*(time+5));
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
      displayTime(time);
    }
    if (key>47) {
      if(count==0){
        lcd.setCursor(0,0);
      }else if(count==2){
        lcd.blink();
        lcd.setCursor(3,0);
      }else if(count==4){
        lcd.blink();
        lcd.setCursor(5,0);
      }
      *time = key-48;     //assign key value to array askii replacement
      lcd.print(*time); //printing value entered on screen
      time += 1;    //incrementing addr of time to get next addr of array
      count++;      
    }  
  }
  delay(1000);
  lcd.clear();
  lcd.noBlink();
  lcd.print("Done!!");
  delay(1000);
  lcd.clear();
  return true;
}

/*//BCD to INT conversion untested
int BCDtoINT(byte val){
    return (int)((val/16*10)+(val%16));
  }
  */
