#include <Servo.h>
#define SERVO_PIN 10

/*サーボモーターの仕組み
橙色の線にPWM信号を入力することで角度を指定できる
PWM信号の決まり
・周波数50Hz(20msサイクル=20000us)
・HIGH時間:(0.5~2.4ms=500us~2400us)
HIGHの時間が0.5msなら-90度, 2.4msなら90度になるまで回転する
回転の速さは約60度/0.1s
*/
Servo motor;

void setup() {
  // put your setup code here, to run once:
  motor.attach(SERVO_PIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  for(int i = 544; i <= 2440; i ++){
    motor.writeMicroseconds(i);
    delay(10);
  }
}
