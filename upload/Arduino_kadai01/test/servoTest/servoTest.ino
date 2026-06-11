//Servo.h相当を作ってみる

#define SERVO_PIN PB1

void myServo(int deg){
  if(-90 <= deg && deg <= 90){
    OCR1A = 2900 + 190 * deg /9 - 1;
  }
}

void setup() {
  Serial.begin(9600);
  DDRB  |= (1 << SERVO_PIN);
  // put your setup code here, to run once:
  TCCR1A = 0B10000010;
  //OC1AでPWM(非反転、比較一致でLOW)
  TCCR1B = 0B00011010;
  //WGM:高速PWM, TOPはICR1/分周比:8
  ICR1   = 40000 - 1;
  
  Serial.println("Enter an integer between -90 to 90");
  myServo(0);
  delay(2000);
}
int inputedDegree = 0;
void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    inputedDegree = Serial.parseInt();
    Serial.println("Enter an integer between -90 to 90");
    if(inputedDegree < -90 || 90 < inputedDegree){
      inputedDegree = 0;
    }
  }
  myServo(inputedDegree);
  delay(1000);
}
