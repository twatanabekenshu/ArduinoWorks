const int buzzerPin = 13;

void setup() {
  // put your setup code here, to run once:
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  tone(buzzerPin, 269.292);
  delay(500);
  noTone(buzzerPin);
  delay(500);
  tone(buzzerPin, 302.270);
  delay(500);
  noTone(buzzerPin);
  delay(500);
  tone(buzzerPin, 339.286);
  delay(500);
  noTone(buzzerPin);
  delay(1000);

}
