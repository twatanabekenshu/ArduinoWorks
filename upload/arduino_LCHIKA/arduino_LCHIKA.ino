int ledPins[2] = {2, 4};

void setup() {
  // put your setup code here, to run once:
  for(int i = 0; i < 2; i ++){pinMode(ledPins[i], OUTPUT);}
}

int ledState = 0;

void loop() {
  // put your main code here, to run repeatedly:
  for(int i = 0; i < 2; i ++){digitalWrite(ledPins[i], ledState);}
}
