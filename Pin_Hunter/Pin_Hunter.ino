
int tempswitch30 = 3;
int callfan = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //Start Serial
pinMode(tempswitch30, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
callfan=digitalRead(tempswitch30);
Serial.println(digitalRead(tempswitch30));
if (callfan == HIGH) {
  Serial.println ("Temp above 30");
} else {
  Serial.println ("Temp below 30");
}
Serial.println ();
delay (1000);
}
