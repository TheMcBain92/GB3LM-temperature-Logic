/*
  LCD Temp Sensor logger

  Logs temps to SD Card + LCD

  The circuit:
  - LCD and RTC connected Analog pin4 + pin5 I2c bus
  - Temp Sensors OneWire to Digital pin2 (4.7K Resistor between VCC and Data Line)
  - SD Card -MISO pin12
            -MOSI pin11
            -SCK pin13
            -CS pin10 Changable, See //SetupSD below.

  created 2018
  by Stephen McBain <http://mcbainsite.co.uk>
*/
const char *codeversion = "V0.0.10beta";
//Load needed libries / Basic Setup
#include <SPI.h>
#include <SD.h>
  // -- Set SD Card Variable Name
File dataFile;

#include <Wire.h> //I2C Communication
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h>
//#include <DS3231.h>
// -- Init the DS3231 using the hardware interface
//DS3231  rtc(SDA, SCL);

#include <OneWire.h>
#include <DallasTemperature.h>
// -- Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
// -- Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// -- Assign sensor addresses to Variable Names
// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
DeviceAddress VHF = { 0x28, 0xFF, 0xCC, 0xD3, 0x31, 0x18, 0x02, 0x91 }; 
DeviceAddress IN = { 0x28, 0xFF, 0x36, 0x54, 0xC4, 0x17, 0x05, 0xF6 };
DeviceAddress OUT = { 0x28, 0xFF, 0xE1, 0x44, 0xC4, 0x17, 0x04, 0x9F };
DeviceAddress HF = { 0x28, 0xFF, 0x3D, 0xEA, 0x31, 0x18, 0x02, 0x21 };
DeviceAddress ATV = { 0x28, 0xFF, 0xCF, 0x53, 0x32, 0x18, 0x01, 0x00 };
//DeviceAddress Sensor6 = {};

// -- LCD Setup Assign pins to names
#define I2C_ADDR 0x27  //Address location of I2C LCD
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
DS3232RTC RTC;

//Setup LCD Pins etc...
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

//Set temp allert marker temprature
const PROGMEM int allert = 50;

int SDpresent = 0; // -- Initialze variable for SD logging
int myh = 0; // -- Variable to set 12 hour time
//int t1 = 0; // -- Test temp from RTC
float S1;
float S2;
float S3;
float S4;
float S5;
//float S6;
//float temp;
long previousMillis = 0;
long interval = 10000;
byte startup = 1;
const byte interruptPin = 3;

void setup()
{
  Serial.begin(9600); //Start Serial
  // -- Set Real Time Clock. Run once then // comment out
//setTime(22, 37, 45, 02, 12, 2019);   //set the system time to 23h31m30s on 13Feb2009
//RTC.set(now());                     //set the RTC from the system time
  // -- Check Real Time Clock Is Set
  setSyncProvider(RTC.get);  // Library function to get the time from the RTC module. DS3232RTC.h Library
  if (timeStatus() != timeSet)
    Serial.println(F("System Time Cannot be Set. Check Connections."));
  else
    Serial.println(F("System Time is Set."));
    Serial.print(day());
    Serial.print("/");
    Serial.print(month());
    Serial.print("/");
    Serial.print(year());
    Serial.print(", ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());
  
  lcd.begin (20, 4); //LCD Size (horrizontal, Vertical) 
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), test, FALLING);
  // LCD Backlight ON
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);

  // Start up the sensors library
  Serial.print(F("Locating devices..."));
  sensors.begin();
  Serial.print(F("Found "));
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(F(" devices."));

  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(11);
  //sensors.setResolution(IN, 10);
  //sensors.setResolution(OUT, 10);
  //sensors.setResolution(HF, 10);
  //sensors.setResolution(ATV, 10);
  //sensors.setResolution(S6, 10);

  // -- LCD Welcome Message
  lcd.setCursor(1, 0); // go home on LCD
  lcd.print("Shack Temp Monitor");
  lcd.setCursor(7, 1);
  lcd.print(codeversion);
  lcd.setCursor(2, 3);
  lcd.print("mcbainsite.co.uk");
  delay(3000);

  //SetupSD Card
  Serial.print(F("Initializing SD card..."));
  lcd.clear();
  lcd.home();
  lcd.print(F("Initializing Log..."));
  lcd.setCursor(0, 2);
  lcd.print(F("SD Card..."));

  // -- Check for SD Card and Activate Logging
  // -- Change 10 below to Set CS Pin
  if (!SD.begin(10)) {
    Serial.println(F("SD inisalisation Failed"));
    lcd.print(F("ERROR!"));
    delay(2000);
    //headderNOSD();
    layout();
  } else {
    SDpresent = 1;
    dataFile = SD.open("startup.txt", FILE_WRITE);
    dataFile.print("Startup logged.....");
    Serial.print(F("Writing headder...."));
    dataFile.println("Shack Temp Monitor");
    dataFile.println(codeversion);
    dataFile.println("Written By Stephen McBain");
    dataFile.println();
    dataFile.print(day());
    dataFile.print(month());
    dataFile.println(year());
    dataFile.println();
    dataFile.close();
    Serial.println(F("SD Ready"));
    lcd.print(F("Ready!"));
    delay(2000);
    //headder();
    layout();
  }
}

void headder() //Setup LCD with logging active message
{
  Serial.println(F("Headder With Logging"));
  lcd.clear();
  lcd.home();
  lcd.print(F("Temp Log Active"));
  layout();
}

void headderNOSD() //Setup LCD with logging inactive message
{
  Serial.println(F("Headder With no Logging"));
  lcd.clear();
  lcd.home();
  lcd.print("Temp Log inActive");
  delay(2000);
  layout();
}

void layout()// standard layout of screen
{ 
  lcd.clear();
  currentTD();
  delay(3000);
  //lcd.clear();
  //int t1 = RTC.temperature(); // retrieve the value from the DS3232
  //float temp = t1 / 4; // temperature in Celsius stored in temp
  //lcd.print(temp);
  //delay(3000);
  startup = 0;
  lcd.clear();
  lcd.home();
  lcd.print(F("The McBain Temp Log"));
  lcd.setCursor(0, 1);
  lcd.print(F("IN :"));
  lcd.setCursor(10, 1);
  lcd.print(F("OUT:"));
  lcd.setCursor(0, 2);
  lcd.print(F("HF :"));
  lcd.setCursor(10, 2);
  lcd.print(F("VHF:"));
  lcd.setCursor(0, 3);
  lcd.print(F("ATV:"));
  //lcd.setCursor(10, 3);
  //lcd.print(F("S6 :N/A"));
  loop;
}

void currenttime()
{
//24h Convert
if(hour() >= 13) {
  myh = hour() - 12;
} else {
  myh = hour();
}

//Print Hour
if(myh <= 9) {
  lcd.print(" ");
}
lcd.print(myh);
lcd.print(":");

//Print Minute
if(minute() <= 9) {
  lcd.print("0");
}
lcd.print(minute());

//Print Second
if(!startup == 1) {
  lcd.print(":");
  if(second() <= 9) {
    lcd.print("0");
  }
lcd.print(second());
}

//print am or pm
if(hour() >= 12) {
  lcd.print("pm");
} else {
  lcd.print("am");
}

return;
}

void currentdate()
{
  if(day() < 10) {
    lcd.print("0");
  }
  lcd.print(day());
  lcd.print("/");
  if(month() < 10) {
    lcd.print("0");
  }
  lcd.print(month());
  lcd.print("/");
  lcd.print(year());
  return;
}

void currentTD()
{
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Current Time");
  lcd.setCursor(6, 2);
  currenttime();
  lcd.setCursor(5, 3);
  currentdate();
  return;
}

void test()
{
  noInterrupts();
  Serial.print("Interupt");
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Current Time");
  lcd.setCursor(6, 2);
  currenttime();
  lcd.setCursor(5, 3);
  currentdate();
  delay(5000);
  interrupts();
  layout();
}

void tempreadprint()
{
  Serial.println(F("Reading Temps and Printing to LCD"));
//sensors.requestTemperatures(); // Send the command to get temperature readings
sensors.requestTemperaturesByAddress(VHF); // Send the command to get temperatures
sensors.requestTemperaturesByAddress(IN); // Send the command to get temperatures
sensors.requestTemperaturesByAddress(OUT); // Send the command to get temperatures
sensors.requestTemperaturesByAddress(HF); // Send the command to get temperatures
sensors.requestTemperaturesByAddress(ATV); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(tempSensor); // Send the command to get temperatures

float S1 = sensors.getTempC(IN);
float S2 = sensors.getTempC(OUT);
float S3 = sensors.getTempC(HF);
float S4 = sensors.getTempC(VHF);
float S5 = sensors.getTempC(ATV);
//float S6 = sensors.getTempCByIndex(5);

  //Sensor 1
  lcd.setCursor(4, 1);
  if (S1 == -127.00) {
    lcd.print("N/A   ");
  } else {
    if (S1 >= allert) {
      lcd.print(S1); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print("*");
    } else {
      lcd.print(S1); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print(" ");
    }
  }
  //Sensor 2
  lcd.setCursor(14, 1);
  if (S2 == -127.00) {
    lcd.print("N/A   ");
  } else {
    if (S2 >= allert) {
      lcd.print(S2); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print("*");
    } else {
      lcd.print(S2); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print(" ");
    }
  }
  //Sensor 3
  lcd.setCursor(4, 2);
  if (S3 == -127.00) {
    lcd.print("N/A   ");
  } else {
    if (S3 >= allert) {
      lcd.print(S3); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print("*");
    } else {
      lcd.print(S3); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print(" ");
    }
  }
  //Sensor 4
  lcd.setCursor(14, 2);
  if (S4 == -127.00) {
    lcd.print("N/A   ");
  } else {
    if (S4 >= allert) {
      lcd.print(S4); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print("*");
    } else {
      lcd.print(S4); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print(" ");
    }
  }
    //Sensor 5
  lcd.setCursor(4, 3);
  if (S5 == -127.00) {
    lcd.print("N/A   ");
  } else {
    if (S5 >= allert) {
      lcd.print(S5); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print("*");
    } else {
      lcd.print(S5); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
      lcd.print(" ");
    }
  }
  //Sensor 6
//  lcd.setCursor(14, 3);
//  if (S6 == -127.00) {
//    lcd.print("N/A   ");
//  } else {
//    if (S6 >= allert) {
//      lcd.print(S6); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
//      lcd.print("*");
//    } else {
//      lcd.print(S6); // Why "byIndex"?
      // You can have more than one DS18B20 on the same bus.
      // 0 refers to the first IC on the wire
//      lcd.print(" ");
//    }
//  }
return;
}

void sdtempwrite()
{
  if (SDpresent == 1) {
    if (((minute() == 00) && (second() >= 29 && second() <= 30)) || ((minute() == 10) && (second() >= 29 && second() <= 30)) || ((minute() == 20) && (second() >= 29 && second() <= 30)) || ((minute() == 30) && (second() >= 29 && second() <= 30)) || ((minute() == 40) && (second() >= 29 && second() <= 30)) || ((minute() == 50) && (second() >= 29 && second() <= 30))) {
      //sensors.requestTemperatures(); // Send the command to get temperature readings
      /********************************************************************/
      Serial.println(F("Data write"));
      String filename = String(day()) + "-" + String(month()) + "-" + String(year()) + ".csv";
      char str[16] = {0};
      filename.toCharArray(str, 16);
      dataFile = SD.open(str, FILE_WRITE);
      dataFile.print(day());
      dataFile.print("/");
      dataFile.print(month());
      dataFile.print("/");
      dataFile.print(year());
      dataFile.print(",");
      dataFile.print(hour());
      dataFile.print(":");
      dataFile.print(minute());
      dataFile.print(":");
      dataFile.print(second());
      dataFile.print(",");
      dataFile.print("VHF:");
      dataFile.print(",");
      dataFile.print(S1);
      dataFile.print(",");
      dataFile.print("IN:");
      dataFile.print(",");
      dataFile.print(S2);
      dataFile.print(",");
      dataFile.print("OUT:");
      dataFile.print(",");
      dataFile.print(S3);
      dataFile.print(",");
      dataFile.print("HF:");
      dataFile.print(",");
      dataFile.print(S4);
      dataFile.print(",");
      dataFile.print("ATV:");
      dataFile.print(",");
      dataFile.println(S5);
//      dataFile.print(",");
//      dataFile.print("S6:");
//      dataFile.print(",");
//      dataFile.println(S6);
      dataFile.close();
    }
  }
return;
}

void loop()
{
lcd.setCursor(10, 3);
currenttime();
unsigned long currentMillis = millis();
if(currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;
    tempreadprint();
    //sdtempwrite();
  }
}
