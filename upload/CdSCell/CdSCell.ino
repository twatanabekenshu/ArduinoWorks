int inputPin = 0;
int inputValue = 0;
int ledPin = 4;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //inputValue = analogRead(inputPin);
  //暗くなったらおおむね700未満になることから暗くなったら点灯するようになっている
  while(analogRead(inputPin) < 700){
    digitalWrite(ledPin, HIGH);
  }
  digitalWrite(ledPin, LOW);
}
