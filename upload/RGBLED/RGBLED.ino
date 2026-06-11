int redPin = 3; 
int grnPin = 5;
int bluPin = 6;

void setup() {
  // put your setup code here, to run once:
  DDRD |= 0b01101000;
}

void loop() {
  // put your main code here, to run repeatedly:
  PORTD = 0b00001000;
  delay(1000);
  PORTD = 0b00101000;
  delay(1000);
  PORTD = 0b00100000;
  delay(1000);
  PORTD = 0b01100000;
  delay(1000);
  PORTD = 0b01000000;
  delay(1000);
  PORTD = 0b01001000;
  delay(1000);
}
