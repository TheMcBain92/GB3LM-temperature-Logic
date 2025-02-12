# GB3LM-temperature-Logic
 Logic for the temperature managment of GB3LM

Required Libraires:

Dalas Instruments Library:
https://codeload.github.com/milesburton/Arduino-Temperature-Control-Library/zip/master

One Wire Library:
https://halckemy.s3.amazonaws.com/uploads/attachments/229743/OneWire.zip

DS3231 RTC Library:
https://codeload.github.com/NorthernWidget/DS3231/zip/master

DS3232RTC Library:
https://codeload.github.com/JChristensen/DS3232RTC/zip/master

NewliquidCrystal Library:
https://bbuseruploads.s3.amazonaws.com/e2740454-6996-43a6-8eb1-4cbdaae86ddd/downloads/3c6d068b-55d6-444f-894a-0a866fda1300/Newliquidcrystal_1.3.5.zip?Signature=5qxxyhsZh8BbckqwrB7skBkp2%2FQ%3D&Expires=1525280388&AWSAccessKeyId=AKIAIQWXW6WLXMB5QZAQ&versionId=rmBp8cCA.FY1Biapj2YIUssJDM8_PB3I&response-content-disposition=attachment%3B%20filename%3D%22Newliquidcrystal_1.3.5.zip%22

/*
  The circuit:
  - LCD and RTC connected Analog pin4 (SCL) + pin5 (SDA) I2c bus
  - Temp Sensors OneWire to Digital pin2 (4.7K Resistor between VCC and Data Line)
  created 2025
  by Stephen McBain <https://mcbainsite.co.uk>
*/
