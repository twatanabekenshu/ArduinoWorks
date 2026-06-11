/*
Arduino標準のtone()関数, noTone()関数を作ってみる
計算上、分周比を64か256で分けなければならない、割込必要
3pinでの実装に挑戦してみる3pin(OC2B)はPWMを使えるので実装が簡単
他のピンの場合、割込が必要

*/

#define BUZZER_PIN PD3

void myNoTone(){
  //
  TCCR2B &= 0B11111000;
  PORTD  &= ~(1 << BUZZER_PIN);
  OCR2B   = 0;
}

void myTone(double freq){
  /*
  timer2の分周比とOCR2Aを設定する:
  OCR2Aは次の計算で求められる
  OCR2A = (クロック周波数)/(分周比)/(音の周波数)
  0 < OCR2A < 256でなければならないことに注意
  ここではOCR2A = 250を下限とし、50を上限とする
  分周比  クロック周波数/分周比   周波数下限   周波数上限
     1               16000000     64000      320000   　全域が可聴域外
     8                2000000      8000       40000
    32                 500000      2000       10000
    64                 250000      1000        5000
   128                 125000       500        2500
   256                  62500       250        1250
  1024                  15625        62.5       312.5
  */
  //可聴域外なので何もしない
  if(freq > 25000){
    myNoTone();
    return;
  }
  if(freq > 8000){
    //分周比8
    TCCR2B = 0B00001010;
    OCR2A = 2000000/freq-1;
    OCR2B = OCR2A/2;
    return;
  }

  if(freq > 2000){
    //分周比32
    TCCR2B = 0B00001011;
    OCR2A = 500000/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  //分周比64は対応範囲が狭いため使用しない

  if(freq > 500){
    //分周比128
    TCCR2B = 0B00001101;
    OCR2A = 125000/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  if(freq > 250){
    //分周比256
    TCCR2B = 0B00001110;
    OCR2A = 62500/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  if(freq > 62.5){
    //分周比1024
    TCCR2B = 0B00001111;
    OCR2A = 15625/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  //範囲外//音を出さない
  myNoTone();
  return;
}

void setup() {
  //BUZZER_PINを出力に設定し、LOWにしておく
  DDRD |= (1 << BUZZER_PIN);
  PORTD&= ~(1 << BUZZER_PIN);
  //3pinでのPWMモードに設定
  TCCR2A= 0B00100011;
  //そのうえでWGM2を111に設定しておく
  TCCR2B= 0B00001000;
}

void loop() {
  // put your main code here, to run repeatedly:
  myTone(349.228);
  delay(500);
  myTone(391.995);
  delay(450);
  myTone(440.000);
  delay(400);
  myTone(466.164);
  delay(350);
  myTone(523.251);
  delay(300);
  myTone(587.330);
  delay(250);
  myTone(659.255);
  delay(200);
  myTone(698.456);
  delay(150);
  myTone(783.991);
  delay(100);
  delay(400);
  myNoTone();
  delay(1000);
  
  for(int i = 0; i < 10; i ++){
    myTone(220.000);
    delay(100);
  }
  myNoTone();
  delay(1000);

/*
349.228,
369.994,
391.995,
415.305,
440.000,
466.164,
493.883,
523.251,
554.365,
587.330,
622.254,
659.255,

*/}
