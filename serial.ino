#include <Sleep_n0m1.h>
#include <Wire.h>                                // I2C functions
#include <SPI.h>
#include <SD.h>	                                 // the SD Card library
#include <SoftwareSerial.h>                      // soft serial ports
#include <EEPROM.h>                              // built-in EEPROM routines


int   Pingpower = 3;                             // power to Ping sensor
int   SDpower = 9;                               // power to microSD card circuit
int   SD_CS = 10;                                // ChipSelect pin for SDcard SPI

SoftwareSerial echoSerial =  SoftwareSerial(4, 5, true);  //RX,TX, inverted?
Sleep sleep;
File myfile;                                     // initialize SDcard

char c;
double distance;
unsigned long sleepTime = 500;                         //how long you want the arduino to sleep
int   IDnum;                                     // Board/Site identifier
char  name[] = "XXX-xxx.txt";                    // SDcard file
  char  a[4];                                      // buffer array for itoa function 


int   DS1337ctrl = B1101000;                     // DS1337 i2c identifier
int   rtc_bcd[8];                                // time, date values
int   rtc[8];
int   secs;
int   mins;
int   hrs;
int   dow;
int   days;
int   mnths;
int   yrs;

int   menuinput;                                 // user input for menu 
long  timeout;                                   // seconds before menu times out
int   indata;                                    // user-input data
int   input;
int   addr;                                      // EEPROM address
byte  EEPROMbyte;                                // EEPROM data to store/read


void setup() {
  int i;
  delay(100);
  
  pinMode(SDpower, OUTPUT);                      // power for SDcard
  pinMode(SD_CS,OUTPUT);
  
  Wire.begin();                                  // enable i2c bus

  Serial.begin(9600);
  echoSerial.begin(9600);

  digitalWrite(SDpower,HIGH);		     // turn on SDcard LED at startup to verify operation
  delay(1500);				     // wait

  if (!SD.begin(SD_CS))                      // check SD card
  {
    Serial.println("initialization failed!");

    for(i=1; i<7  ; i++)                     // blink LED as warning
    {
      digitalWrite(SDpower,HIGH);
      delay(150);
      digitalWrite(SDpower,LOW);
      delay(150);
    }
  }
  digitalWrite(SDpower,LOW);
  
  //menu();                                        // goto menu

}

double get_distance() {
  int i;
  double dist;
  double once;
  
  pinMode(Pingpower,OUTPUT);                     // power for Ping
  digitalWrite(Pingpower,HIGH);               // turn on power to sensor
  
  delay(200);
  
  for(i=1; i<=10; i++) {
    once = ping();
    dist += ping();
  }
  
  digitalWrite(Pingpower, LOW);

  return (dist/10.0);
}

double ping() {
  char dist[6];
  int state = 0;
  int j = 0;
  dist[j] = 0;
  
  while (state < 3) {
    if(echoSerial.available()) {
      c = echoSerial.read();

      if (c == 'R') {
        state = 1;
      } else if (c == 13) {
        state = 3;
      } else if (state = 1) {
        state = 2;
      }
      if (state == 2) {
        dist[j]  = c;
        dist[j+1] = 0;
        j++;
      }
    }
  }
  return atof(dist);
}

//---------- read DS1337 RTC --------------------------------
void readRTC()
{
  Wire.beginTransmission(DS1337ctrl);            // send i2c control identifier
  Wire.write(0x00);                               // set register to 0
  Wire.endTransmission();

  Wire.requestFrom(DS1337ctrl, 7);               // read 7 bytes: secs,min,hr,dow,day,mth,yr
  rtc_bcd[1] = Wire.read();
  rtc_bcd[2] = Wire.read();
  rtc_bcd[3] = Wire.read();
  rtc_bcd[4] = Wire.read();
  rtc_bcd[5] = Wire.read();
  rtc_bcd[6] = Wire.read();
  rtc_bcd[7] = Wire.read();
  Wire.endTransmission();

  for(int i=1; i<=7; i++)                        // convert BCD to decimal
  {
    rtc[i] = (rtc_bcd[i]/16)*10 + rtc_bcd[i]%16;
  }

  secs = rtc[1];
  mins = rtc[2];
  hrs = rtc[3];
  days = rtc[5];
  mnths = rtc[6];
  yrs = rtc[7];
  yrs = yrs + 2000;                              // change year to 20xx format
}
//===================================================================

//-------- print data to screen ---------------------------------
void printData()
{
  Serial.print(IDnum);                           // Site identifier
  Serial.print(' ');
  Serial.print(mnths);                           // date
  Serial.print('-');
  Serial.print(days);
  Serial.print('-');
  Serial.print(yrs);
  Serial.print(' ');

  Serial.print(hrs);                             // time
  Serial.print(':');
  if(mins < 10) Serial.print('0');
  Serial.print(mins);
  Serial.print(':');
  if(secs < 10) Serial.print('0');
  Serial.print(secs);
  Serial.print(' ');
  
  Serial.print(distance);                      // distance, uncorrected
  Serial.println();
  delay(10);
}
//===============================================================

// ------- store data to SDcard ---------------------------------------
void storeData()
{
  
  pinMode(SDpower, OUTPUT);                      // power for SDcard
  pinMode(SD_CS,OUTPUT);
  
  digitalWrite(SDpower,HIGH);                    // turn on power to SDcard
  delay(500);

  if (!SD.begin(SD_CS))                      // check SD card
  {

  }

  myfile = SD.open(name, FILE_WRITE);
  delay(10);

  myfile.print(IDnum);                             // Site identifier
  myfile.print(' ');
  myfile.print(mnths);                             // date
  myfile.print('-');
  myfile.print(days);
  myfile.print('-');
  myfile.print(yrs);
  myfile.print(' ');

  myfile.print(hrs);                               // hour
  myfile.print(':');
  if(mins < 10) myfile.print('0');
  myfile.print(mins);
  myfile.print(':');
  if(secs < 10) myfile.print('0');
  myfile.print(secs);
  myfile.print(' ');

  myfile.print(distance);                        // distance, uncorrected

  myfile.println();

  myfile.close();                                  // close file, write to SDcard
  delay(200);                                      // give time to finish  

  digitalWrite(SDpower,LOW);                       // turn off SDcard power
}
//==========================================================================


void loop() {
  // put your main code here, to run repeatedly:
 
  readRTC();                                     // read RTC

  distance = get_distance();
  printData();                                 // output data to screen
  storeData();                                 // store data to SD card
    
  //delay(1000);
  
  //delay(sleepTime);
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(sleepTime); //sleep for: sleepTime

}

//--------------------------------------------------------
void menu()
{

  addr = 1;                                      // read ID number from EEPROM
  EEPROMbyte = EEPROM.read(addr);                // high byte
  IDnum = EEPROMbyte * 256;
  addr ++;                                       // next address
  EEPROMbyte = EEPROM.read(addr);                // low byte
  IDnum += EEPROMbyte;                           // combine

    itoa(IDnum,a,10);                            // convert ID number to character array
  name[4] = a[0];                                // put into file name[] array
  name[5] = a[1];
  name[6] = a[2];

  Serial.println();                              // print out board info
  Serial.print("Board number:  ");               // IDnum
  Serial.println(IDnum);

  Serial.print("Data filename: ");               // SD filename
  Serial.println(name);

  readRTC();
  Serial.print("Current date:  ");
  Serial.print(mnths);                           // date
  Serial.print('-');
  Serial.print(days);
  Serial.print('-');
  Serial.println(yrs);

  Serial.print("Current time:  ");
  Serial.print(hrs);                             // time
  Serial.print(':');
  if(mins < 10) Serial.print('0');
  Serial.print(mins);
  Serial.print(':');
  if(secs < 10) Serial.print('0');
  Serial.println(secs);

  Serial.println();                              // menu options
  Serial.println("1. Set clock");
  Serial.println("2. Set ID number");
  Serial.println();

  timeout = millis() + 5000;                     // wait for input before timing out
  while(millis() < timeout)
  {
    if(Serial.available())                       // if something typed, go to menu
    {
      menuinput = Serial.read();
      break;
    }
  }

  menuinput -= 48;                               // convert ASCII to decimal
  switch(menuinput)
  {
  case 1:          //-------------------------------------------------------
    Serial.println("Set clock:");                // set RTC

    Serial.print("  input month:  ");
    getinput();
    mnths = indata;
    Serial.print("  input day:    ");
    getinput();
    days = indata;
    Serial.print("  input year:   ");
    getinput();
    yrs = indata;
    Serial.print("  input hour:   ");
    getinput();
    hrs = indata;
    Serial.print("  input minute: ");
    getinput();
    mins = indata;

    mins = (mins/10)*16 + mins%10;               // convert decimal to BCD
    hrs = (hrs/10)*16 + hrs%10;
    days = (days/10)*16 + days%10;
    mnths = (mnths/10)*16 + mnths%10;
    yrs = (yrs/10)*16 + yrs%10;

    Wire.beginTransmission(DS1337ctrl);          // set time, date
    Wire.write(0);

    Wire.write(secs);
    Wire.write(mins);
    Wire.write(hrs);
    Wire.write(dow);
    Wire.write(days);
    Wire.write(mnths);
    Wire.write(yrs);

    Wire.endTransmission();

    break;

  case 2:          // ----------------------------------------------------
    Serial.println("Set board ID number:");      // set board IB number

    Serial.print("  input ID number: ");
    getinput();
    IDnum = indata;

    addr = 1;                                    // store IDnum in EEPROM
    EEPROMbyte = IDnum / 256;                    // high byte
    EEPROM.write(addr,EEPROMbyte);
    delay(20);
    addr ++;
    EEPROMbyte = IDnum % 256;                    // low byte
    EEPROM.write(addr,EEPROMbyte);
    delay(20);

    break;
  }

  int k;
  for(k=1; k<=5; k++)                            // print out 5 readings
  {
    readRTC();
    /* readLM35(); */
    ping();
    printData();
    delay(2000);
  }
  Serial.println();
  Serial.println("ok ...");
  Serial.println();
  delay(5);
}
//================================================================


//----------------- read user input -----------------------------------------------
void getinput()
{
  int i;
  timeout = millis() + 5000;                     // period to wait before timing out       

  indata = 0;
  while(millis() < timeout)
  {
    i = 0;
    if(Serial.available())                       // if something availabe from serial port
    {
      input = Serial.read();                     // read input
      if(input == 13) break;                     // if carraige return, done
      if(input > 47 and input < 58)              // look for number between 0 and 9
      {
        input -= 48;                             // convert ASCII input to decimal value
        Serial.print(input);
        delay(5);
        indata = indata*10 + input;              // concatenate multiple inputs
      }
      else
      {
        name[i] = input;
        Serial.print(input);
        i++;
      }
    }
  }
  Serial.println();
  delay(5);
}
//===================================================================

