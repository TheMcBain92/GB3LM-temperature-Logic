/*
  GB3LM Temperature Control Logic

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
DS3232RTC RTC; //Declare Access name for RTC Library
#include <OneWire.h>
#include <DallasTemperature.h>
// -- Data Pin OneWire Devices connected too
#define ONE_WIRE_BUS 2
// -- Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// -- Assign sensor addresses to Variable Names
// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
DeviceAddress TX = { 0x28, 0xFF, 0x6B, 0x13, 0x32, 0x18, 0x01, 0xCC };
DeviceAddress RX = { 0x28, 0xFF, 0x16, 0xBC, 0x31, 0x18, 0x02, 0x78 };
DeviceAddress AMB = { 0x28, 0xFF, 0xE1, 0x22, 0x32, 0x18, 0x01, 0x7D };

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
float S6;
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

  // -- Set Real Time Clock. Run once then // comment out
//setTime(23, 31, 30, 09, 02, 2025);   //set the system time to 23h31m30s on 09Feb2025
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
  Serial.println(F("Locating devices..."));
  sensors.begin();
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  while(oneWire.search(addr)) {
    Serial.print("\r\nFound Sensor with address:\r\n");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid\r\n");
        return;
    }
  }
  Serial.print(F("\r\n\r\nFound "));
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(F(" devices.\r\n"));
  oneWire.reset_search();

  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(10);
  //sensors.setResolution(IN, 10);
  //sensors.setResolution(OUT, 10);
  //sensors.setResolution(HF, 10);
  //sensors.setResolution(ATV, 10);
  //sensors.setResolution(S6, 10);

  // -- LCD Welcome Message
  lcd.setCursor(1, 0); // go home on LCD
  lcd.print("GB3LM Logic");
  lcd.setCursor(7, 1);
  lcd.print("Version ");
  lcd.print(codeversion);
  lcd.setCursor(2, 3);
  lcd.print("mcbainsite.co.uk");
  delay(3000);
  Serial.print("------Setup Completed------\r\n\r\n");
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
//sensors.requestTemperatures(); // Send the command to get temperature readings
sensors.requestTemperaturesByAddress(TX); // Send the command to get temperatures
sensors.requestTemperaturesByAddress(RX); // Send the command to get temperatures
sensors.requestTemperaturesByAddress(AMB); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(TBC1); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(TBC2); // Send the command to get temperatures
//sensors.requestTemperaturesByAddress(TBC3); // Send the command to get temperatures
float S1 = sensors.getTempC(TX);
float S2 = sensors.getTempC(RX);
float S3 = sensors.getTempC(AMB);
//float S4 = sensors.getTempC(TBC1);
//float S5 = sensors.getTempC(TBC2);
//float S6 = sensors.getTempC(TBC3);
Serial.println(F("Reading Temps and Printing to LCD"));
  //Sensor 1
  lcd.setCursor(4, 1);
  if (S1 == -127.00) {
    lcd.print("N/A   ");
    Serial.print("TX N/A");
  } else {
    Serial.print(F("TX "));
    Serial.print(S1);
    Serial.println(F(" Degrees"));
    if (S1 >= allert) {
      lcd.print(S1);
      lcd.print("*");
    } else {
      lcd.print(S1);
      lcd.print(" ");
    }
  }
  //Sensor 2
  lcd.setCursor(14, 1);
  if (S2 == -127.00) {
    lcd.print("N/A   ");
    Serial.print("RX N/A");
  } else {
    Serial.print(F("RX "));
    Serial.print(S2);
    Serial.println(F(" Degrees")); 
    if (S2 >= allert) {
      lcd.print(S2);
      lcd.print("*");
    } else {
      lcd.print(S2);
      lcd.print(" ");
    }
  }
  //Sensor 3
  lcd.setCursor(4, 2);
  if (S3 == -127.00) {
    lcd.print("N/A   ");
    Serial.print("AMB N/A");
  } else {
    Serial.print(F("AMB "));
    Serial.print(S3);
    Serial.println(F(" Degrees"));
    if (S3 >= allert) {
      lcd.print(S3);
      lcd.print("*");
    } else {
      lcd.print(S3);
      lcd.print(" ");
    }
  }
  Serial.println();
  return; // REMOVE THIS LINE AND ONE ABOVE WHEN ADDITIONAL SENSORS ADDED 
  //Sensor 4
  lcd.setCursor(14, 2);
  if (S4 == -127.00) {
    lcd.print("N/A   ");
    Serial.print("TBC N/A");
  } else {
    Serial.print(F("TBC "));
    Serial.print(S4);
    Serial.println(F(" Degrees"));
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
    Serial.print("TBC N/A");
  } else {
    Serial.print(F("TBC "));
    Serial.print(S5);
    Serial.println(F(" Degrees"));
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
Serial.println();
return;
}


void recvWithStartEndMarkers() {
  const byte numChars = 4;
  char receivedChars[numChars];
  boolean newData = false;
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc; 
  int req = 0;
  while (Serial.available() > 0 && newData == false) {
      rc = Serial.read();
      if (recvInProgress == true) {
          if (rc != endMarker) {
              receivedChars[ndx] = rc;
              ndx++;
              if (ndx >= numChars) {
                  ndx = numChars - 1;
              }
          }
          else {
              receivedChars[ndx] = '\0'; // terminate the string
              recvInProgress = false;
              ndx = 0;
              newData = true;
          }
      }
      else if (rc == startMarker) {
          recvInProgress = true;
      }
  }
  if (strcmp(receivedChars, "TX") == 0) {
    req = 1;
  } else if (strcmp(receivedChars, "RX") == 0) {
    req = 2;
  } else if (strcmp(receivedChars, "AMB") == 0) {
    req = 3;
  } else {
    req = 0;
  }
  if (newData == true) {
    newData = false;
    //sensors.requestTemperatures(); // Send the command to get temperature readings
    sensors.requestTemperaturesByAddress(TX); // Send the command to get temperatures
    sensors.requestTemperaturesByAddress(RX); // Send the command to get temperatures
    sensors.requestTemperaturesByAddress(AMB); // Send the command to get temperatures
    //sensors.requestTemperaturesByAddress(TBC1); // Send the command to get temperatures
    //sensors.requestTemperaturesByAddress(TBC2); // Send the command to get temperatures
    //sensors.requestTemperaturesByAddress(TBC3); // Send the command to get temperatures
    float S1 = sensors.getTempC(TX);
    float S2 = sensors.getTempC(RX);
    float S3 = sensors.getTempC(AMB);
    //float S4 = sensors.getTempC(TBC1);
    //float S5 = sensors.getTempC(TBC2);
    //float S6 = sensors.getTempC(TBC3);
    switch (req) {
      case 1:
        Serial.println(S1);
        req = 0;
        break;
      case 2:
        Serial.println(S2);
        req = 0;
        break;
      case 3:
        Serial.println(S3);
        req = 0;
        break;
      default:
        Serial.println("Command not recognised");
        req = 0;
        break;
    }
  }
  return;
} 

void loop()
{
lcd.setCursor(10, 3);
currenttime();
recvWithStartEndMarkers();
unsigned long currentMillis = millis();
if(currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;
    tempreadprint();
  }
}
