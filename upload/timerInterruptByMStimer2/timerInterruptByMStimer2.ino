#include <MsTimer2.h>

int cnt = 0;

void timerHandler(){
  Serial.println("3 seconds passed ! ");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Start");
  MsTimer2::set(3000, timerHandler);
  MsTimer2::start();
}

void loop() {
  // put your main code here, to run repeatedly:
  cnt ++;
  Serial.print(cnt);
  Serial.println(" count");
  delay(3500);
}
