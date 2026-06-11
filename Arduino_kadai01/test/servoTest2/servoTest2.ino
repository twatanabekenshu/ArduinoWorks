//Servo.h相当を作ってみる
/*
servoTestからの改良点:
OCR1B(10pin)のPWMを使える(厳密にはPWMではない, タイマー割込関数を使っている)
本当は割込禁止など気を遣わないといけなさそう...
補助カウントを使うことで分周比を小さくしておく. 
*/
#define SERVO_PIN PD4
#define PWM_PIN   PB2 //10pin

#define SUB_CNT_MAX 64
#define ICR1_INITIAL_VALUE 4999
//TCNT1がICR1に一致するごとにインクリメントされ、SUB_CNT_MAXに一致したら0にリセットされる
unsigned int sub_cnt = 0;
unsigned int sub_cnt_off;

void myServo(int deg){
  if(-90 <= deg && deg <= 90){
    unsigned int off_cnt = ((23200 + 1520 * deg /9));
    OCR1A = off_cnt % (ICR1_INITIAL_VALUE+1);
    sub_cnt_off = off_cnt / (ICR1_INITIAL_VALUE+1);
  }
}

void my_PWM_on_OC1B(unsigned int duty){
  if(duty <= ICR1_INITIAL_VALUE){
    OCR1B = duty;
  }
}

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  TCCR1A = 0B00000000;
  //OC1Aは何も出力しない, WGM11,10ともに0にしておく
  TCCR1B = 0B00011001;
  //CTCモード : TCNT1とICR1が一致したら(割り込みを実行してから)TCNT1が0になる
  ICR1   = ICR1_INITIAL_VALUE;
  //タイマー割込 : TCNT1が一致したときとOC1Aが一致したときに割り込み操作を行う
  //OCR1AにはHIGHの時間を対応させておく
  TIMSK1 = 0B00100110;
  //ServoPinを出力に設定
  DDRD  |= (1 << SERVO_PIN); 
  DDRB  |= (1 << PWM_PIN);
  sub_cnt = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  myServo(0);
  delay(1000);
  myServo(90);
  delay(1000);
  myServo(-90);
  delay(1000);
  */
  /*
  for(int i = 0; i < 2000; i ++){
    my_PWM_on_OC1B(i);
    delay(1);
  }
  Serial.println("delay(1000); start");
  delay(1000);
  Serial.println("delay(1000); end");*/
  Serial.println("start");
  my_PWM_on_OC1B(4998);
  delay(1000);
  Serial.println("end");
}

ISR(TIMER1_COMPA_vect){
  //sub_cntとsub_cnt_offが一致するときに限り,PORTDのSERVO_PINをLOWにする
  if(sub_cnt == sub_cnt_off){
    PORTD &= ~(1 << SERVO_PIN);
  }
}

ISR(TIMER1_COMPB_vect){
  //PORTBのPWM_PINをLOWにする
  PORTB &= ~(1 << PWM_PIN);
}

ISR(TIMER1_CAPT_vect){
  PORTB |= (1 << PWM_PIN);
  sub_cnt ++;
  //PORTDのSERVO_PINをHIGHに
  if(sub_cnt >= SUB_CNT_MAX){
    PORTD |=  (1 << SERVO_PIN);
    sub_cnt = 0;
  }
}
