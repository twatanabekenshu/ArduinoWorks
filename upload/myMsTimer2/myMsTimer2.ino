/*
MsTimer2相当のことをしてみる
MsTimer2でやっていること(予想)
1. 1msごとに割込を発生させ、カウントアップする
2. カウントが一定値に達したら処理を実行してカウンタをリセットする

・実現のための方法
timer2のCTCモードで1msの割込を作る
1ms -> 16000000/1000=16000クロック
16000/分周比(1,8,32,64,128,256,1024)のうち、255以下のものを探す。
すると64,128,256,1024となるが、なるべく整数にしたいので、64または128にする。
どちらでもよいが、分周比を64とする。16000/64=250より、OCR2A=250
ISRでカウントアップ+一定値で処理実行

・用意する変数
カウンタ変数 unsigned long millis_cnt(最大値は2^32-1なので、オーバーフローを気にしなくてOK)
間隔を表す変数 unsigned long interval
関数ポインタ void (*func)(void)
*/

unsigned long millis_cnt = 0;
unsigned long interval = 2000;

void task(){
  Serial.println("task fired");
  PORTB ^= 0B00100000;
}

ISR(TIMER2_COMPA_vect){
  millis_cnt ++;
  if(millis_cnt >= interval){
    millis_cnt = 0;
    task();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  TCCR2A = 0B00000001;
  TCCR2B = 0B00000100;
  TIMSK2 = 0B00000010;
  OCR2A = 249;
  DDRB = 0B00100000;
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(3000);
}
