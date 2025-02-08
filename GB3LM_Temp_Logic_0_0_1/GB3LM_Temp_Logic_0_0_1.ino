/*
  The circuit:
  - LCD and RTC connected Analog pin4 + pin5 I2c bus
  - Temp Sensors OneWire to Digital pin2 (4.7K Resistor between VCC and Data Line)

  created 2025
  by Stephen McBain <https://mcbainsite.co.uk>
*/
const char *codeversion = "0.0.1";
//Load needed libries / Basic Setup
#include <SPI.h>
#include <Wire.h> //I2C Communication
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h>
DS3232RTC RTC;
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
DeviceAddress TX = { 0x28, 0xFF, 0x6B, 0x13, 0x32, 0x18, 0x01, 0xCC }; 
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

//Setup LCD Pins etc...
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

//Set temp allert marker temprature
const PROGMEM int allert = 50;
int myh = 0; // -- Variable to set 12 hour time
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
  Serial.println("GB3LM Temprature Logic");
  Serial.print("Version ");
  Serial.println(codeversion);
  Serial.println();
  Serial.println();
  Serial.println();
  // -- Set Real Time Clock. Run once then // comment out
//setTime(22, 37, 45, 02, 12, 2019);   //set the system time to 23h31m30s on 13Feb2009
//RTC.set(now());                     //set the RTC from the system time
  // -- Check Real Time Clock Is Set
//  setSyncProvider(RTC.get);  // Library function to get the time from the RTC module. DS3232RTC.h Library
//  if (timeStatus() != timeSet)
//    Serial.println(F("System Time Cannot be Set. Check Connections."));
//  else
//    Serial.println(F("System Time is Set."));
//    Serial.print(day());
//    Serial.print("/");
//    Serial.print(month());
//    Serial.print("/");
//    Serial.print(year());
//    Serial.print(", ");
//    Serial.print(hour());
//    Serial.print(":");
//    Serial.print(minute());
//    Serial.print(":");
//    Serial.println(second());
  
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
}

void layout()// standard layout of screen
{ 
  lcd.clear();
  currentTD();
  delay(3000);
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
sensors.requestTemperaturesByAddress(TX); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(IN); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(OUT); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(HF); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(ATV); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(tempSensor); // Send the command to get temperatures

float S1 = sensors.getTempC(TX);
//float S2 = sensors.getTempC(OUT);
//float S3 = sensors.getTempC(HF);
//float S4 = sensors.getTempC(VHF);
//float S5 = sensors.getTempC(ATV);
//float S6 = sensors.getTempCByIndex(5);

  //Sensor 1
  Serial.print(S1);
  Serial.println(" Degrees");
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

void loop()
{
lcd.setCursor(10, 3);
currenttime();
unsigned long currentMillis = millis();
if(currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;
    tempreadprint();
  }
}
