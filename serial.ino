#include <Sleep_n0m1.h>
#include <I2C.h>                                // I2C functions
#include <SPI.h>
#include <SD.h>	                                 // the SD Card library
#include <SoftwareSerial.h>                      // soft serial ports
#include <EEPROM.h>                              // built-in EEPROM routines
#include <RTClib.h>

const int   Pingpower = 3;                             // power to Ping sensor
const int   SDpower = 7;                               // power to microSD card circuit
const int   SD_CS = 10;                                // ChipSelect pin for SDcard SPI

SoftwareSerial echoSerial =  SoftwareSerial(4, 5, true);  //RX,TX, inverted?
Sleep sleep;
RTC_DS1307 rtc;

File myfile;                                     // initialize SDcard

double distance;
double lastDistance;

const unsigned long sleepTime =  3600000;                         //how long you want the arduino to sleep
int   IDnum = 13;                                               // Board/Site identifier
char  name[] = "x-13.csv";                                  // SDcard file

int i;

DateTime now;

const int addr = 3;                                      // EEPROM address
byte  EEPROMbyte;                                // EEPROM data to store/read

void setup() {
  
  // check for id number and set if zero
  EEPROMbyte = EEPROM.read(addr);
  
  if (EEPROMbyte == 255) {
    EEPROM.write(addr, IDnum);
  }
  IDnum = EEPROM.read(addr);
  String filename = String(IDnum);
  filename = filename + ".txt";
  filename.toCharArray(name, sizeof(name));
  
  ADCSRA = 0;                                    // disable ADC
 
  pinMode(SDpower, OUTPUT);                      // power for SDcard
  digitalWrite(SDpower, HIGH);		    // turn on SDcard LED at startup to verify operation
  delay(100);

  I2c.begin();                                  // enable i2c bus
  I2c.timeOut(3000);

  Serial.begin(9600);

  rtc.begin();  
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  }
  now = rtc.now();
  I2c.end();
  printDate();
  Serial.println();

  delay(500);				     // wait

  if (!SD.begin(SD_CS))                      // check SD card
  {
    Serial.println("initialization failed!");

    for (i = 1; i < 7  ; i++)                // blink LED as warning
    {
      digitalWrite(SDpower, HIGH);
      delay(150);
      digitalWrite(SDpower, LOW);
      delay(150);
    }
  }

  digitalWrite(SDpower, LOW);
  digitalWrite(SD_CS,LOW);
  
  Serial.end();
  
  TWCR = bit(TWEN) | bit(TWIE) | bit(TWEA) | bit(TWINT);
  //DDRD = DDRD | B11111100;
   //DDRD = B11111111;
   // set PORTD to low
   PORTD = B00000000;
   //PORTB = B00000000;
   PORTC = B00000000;
   
    sleep.pwrDownMode(); //set sleep mode
    sleep.sleepDelay(5); //sleep for: sleepTime
}

double get_distance() {
  double dist = 0;
  int once = 0;
  int count = 0;
  
  echoSerial.begin(9600);

  pinMode(Pingpower, OUTPUT);                    // power for Ping
  digitalWrite(Pingpower, HIGH);              // turn on power to sensor

  delay(350);
  
  for (i = 0; i < 10; i++) {
    once = ping();
    if (once > 1) {                            // guard against zero readings
      dist += once;
      count += 1;
    }
  }
  lastDistance = ping();

  pinMode(Pingpower, INPUT);                    // power for Ping
  digitalWrite(Pingpower, LOW);
  echoSerial.end();

  if (count > 0) {
  return (dist / count);
  } else {
    return -1;
  }
}

double ping() {
  char dist[6];
  char c;
  int state = 0;
  int j = 0;
  unsigned long startTime = millis();
  for(j=0; j< 6; j++) {
    dist[j]=0;
  }

  j =  0;
  while (state < 3 && (millis() - startTime) < 2000) {
    if (echoSerial.available() ) {
      c = echoSerial.read();
      
      if (c == 'R') {
        state = 1;
      } else if (c == '\r' && state == 2) {
        state = 3;
      } 
      else if (state == 1) {
        state = 2;
      }
      if (state == 2) {
        dist[j]  = c;
        if (j < 6) {
          j++;
        }
      }
    }
  }
 
  return atof(dist);
}

void printDate() {
    Serial.print(now.year());
  Serial.print('-');
  if (now.month() < 10) {Serial.print("0"); }
    Serial.print(now.month());
  Serial.print('-');
  if (now.day() < 10) { Serial.print("0"); }
    Serial.print(now.day());
  Serial.print(' ');
    Serial.print(now.hour());
  Serial.print(':');
  if (now.minute() < 10) { Serial.print("0"); }
    Serial.print(now.minute());
  Serial.print(':');
  if (now.second() < 10) {Serial.print("0"); }
    Serial.print(now.second());
  Serial.print(' ');
}

//-------- print data to screen ---------------------------------
void printData()
{
  Serial.begin(9600);
  delay(100);
  Serial.print(IDnum);                           // Site identifier
  Serial.print(' ');
  
  printDate();
  
  Serial.print(distance);                      // distance, uncorrected
  Serial.print(' ');

  Serial.print(lastDistance);

  Serial.print(' ');
  Serial.print(name);
  Serial.println();
  delay(100);
  Serial.end();
}
//===============================================================

// ------- store data to SDcard ---------------------------------------
void storeData()
{
  delay(500);

SD.begin(SD_CS);
//  if (!SD.begin(SD_CS))  {                    // check SD card
//      Serial.println("initialization failed!");
//
//  }
  
  myfile = SD.open(name, FILE_WRITE);

  myfile.print(IDnum);                             // Site identifier
    myfile.print(',');

  myfile.print(now.year());                           // date
  myfile.print('-');
  if (now.month() < 10) {myfile.print("0"); }
  myfile.print(now.month());
  myfile.print('-');
  if (now.day() < 10) {myfile.print("0"); }
  myfile.print(now.day());
  myfile.print(' ');

  myfile.print(now.hour());                             // time
  myfile.print(':');
  if (now.minute() < 10) {myfile.print("0"); }
  myfile.print(now.minute());
  myfile.print(':');
  if (now.second() < 10) {myfile.print("0"); }
  myfile.print(now.second());

  myfile.print(',');

  myfile.print(distance);                        // distance, uncorrected
  myfile.print(',');
  myfile.print(lastDistance);
  myfile.println();

  myfile.close();                                  // close file, write to SDcard
  delay(500);                                      // give time to finish
  
  digitalWrite(SD_CS,LOW);
}
//==========================================================================

void sleeping() {
   // release TWI bus

   TWCR = bit(TWEN) | bit(TWIE) | bit(TWEA) | bit(TWINT);
  //DDRD = DDRD | B11111100;
   //DDRD = B11111111;
   // set PORTD to low
   PORTD = B00000000;
   //PORTB = B00000000;
   PORTC = B00000000;
   
    sleep.pwrDownMode(); //set sleep mode
    sleep.sleepDelay(sleepTime); //sleep for: sleepTime
}

void loop() {
  distance = get_distance();
  
  I2c.begin();
  I2c.timeOut(3000);
  digitalWrite(SDpower, HIGH);                   // turn on power to SDcard
  delay(10);
  
  now = rtc.now();
  I2c.end();
  storeData();                                 // store data to SD card
  digitalWrite(SDpower, LOW);                      // turn off SDcard power

  printData();                                 // output data to screen
  
  sleeping(); 
}

