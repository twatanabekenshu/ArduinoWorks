/*
PWMではなくピン割込みによるtone関数の実装を行う
ライブラリのtone関数で行っていること
1. CTCモードでOCR2Aによる割り込みを設定する
2. ISR(interrupt service routine)で出力をトグルする
これにより、周波数によらずduty 50%の波形が生成される。

OCR2Aの計算
サイクルは2*(OCR2A + 1)となる。
そのため、
(クロック周波数)/{2*(OCR2A + 1)*(分周比)}=(目標周波数)
という式が成り立つ。これを変形することで
OCR2A = (クロック周波数)/{2*(目標周波数)*(分周比)}-1
      = (8000000/分周比/目標周波数)-1
という式が成り立つことがわかる. 

・レジスタで設定すべき事柄は
OCR2Aを上限とするCTCモード
OCR2Aでの割込み許可
適切な分周比設定
ISR:出力ピンの出力反転

上限:OCR2A=39となる周波数, 下限:OCR2A=199となる周波数
      下限       上限
   1  4000      20000
   8   500       2500
  32   125        625
  64    62.5      312.5
 128    31.25     156.3
 256    15.625     78.1
1024     3.90625   19.5

可聴域は20Hz~20000Hzなのでこの範囲のみ操作することにする. 

TCCR2A = 0B00000010;
TCCR2B = 0B00000***;
*/

#define BUZZER_PIN PD3

void myNoTone(){
  //OCR2Aの割り込みを禁止する
  TIMSK2 &= 0B11111101;
  OCR2A = 0;
}

void myTone(double freq){
  //可聴域外なので音を出さない設定にする
  if(freq > 20000 || freq < 20){
    myNoTone();
    return;
  }

  TCCR2A  = 0B00000010;
  if(freq > 4000){
    //分周比1
    TCCR2B = 0b00000001;
    OCR2A  = 8000000/freq - 1;
  }

  if(freq > 500){
    //分周比8
    TCCR2B = 0b00000010;
    OCR2A  = 1000000/freq - 1;
  }

  if(freq > 125){
    //分周比32
    TCCR2B = 0b00000011;
    OCR2A  =  250000/freq - 1;
  }

  if(freq > 62.5){
    //分周比64
    TCCR2B = 0b00000100;
    OCR2A  =  125000/freq - 1;
  }
  if(freq > 31.25){
    //分周比128
    TCCR2B = 0b00000101;
    OCR2A  =  62500/freq - 1;
  }
  if(freq > 15.625){
    //分周比256
    TCCR2B = 0b00000110;
    OCR2A  =  31250/freq - 1;
  }
  TIMSK2 |= 0B00000010;
}

ISR(TIMER2_COMPA_vect){
  PORTD ^= (1 << BUZZER_PIN);
}

void setup() {
  //BUZZER_PINを出力に設定し、LOWにしておく
  DDRD |= (1 << BUZZER_PIN);
  PORTD&= ~(1 << BUZZER_PIN);
}


#define LOW_Hz  329.628
#define MID_Hz  440.000
#define HIGH_Hz 554.365
void loop() {
  // put your main code here, to run repeatedly:
  /*
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
  
  for(int i = 0; i < 1; i ++){
    myTone(220.000);
    delay(1000);
  }*/
  myNoTone();
  delay(700);
  myTone(LOW_Hz);
  delay(700);
  myTone(HIGH_Hz);
  delay(700); 
  myTone(MID_Hz);
  delay(700);
  myTone(LOW_Hz);
  delay(700);
  myTone(HIGH_Hz);
  delay(700); 
  myTone(MID_Hz);
  delay(700);
  myTone(LOW_Hz);
  delay(700);
  myTone(HIGH_Hz);
  delay(700); 
  myTone(LOW_Hz);
  delay(700);
  myTone(MID_Hz);
  delay(1400);
  myNoTone();
  delay(700);
  myTone(MID_Hz);
  delay(700);
  myTone(LOW_Hz);
  delay(700);
  myTone(HIGH_Hz);
  delay(700);
  myTone(MID_Hz);
  delay(700);
  myTone(LOW_Hz);
  delay(700);
  myTone(HIGH_Hz);
  delay(700);
  myTone(MID_Hz);
  delay(1400);
}
