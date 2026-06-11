const int latchPin = 11;
const int clockPin =  9;
const int dataPin =  12;
/*
シフトレジスタ74HC595を用いたLED制御回路
ピン接続
74HC595:
1~7->LED2~8
  8->GND
 16->5V
 15->LED1
 14->Arduino 12(dataPin)
 13->GND
 12->Arduino 11(latchPin)
 11->Arduino  9(clockPin)
 10->5V
  9->OPEN
74HC595の動作
1. ICの12pin(RCLK)がLOWになっている間、入力を受け付ける. 
2. 14pin(SER)から1ビット分の入力を受け入れる. 
   14pinがHIGHなら1, LOWなら0とする. 
3. ICの11pin(SRCLK)の1クロックで2. で入力したデータを1ビットシフトする
4. ICの12pin(RCLK)がHIGHになったら入力終了

動作例
 RCLK : -____________-
  SER : _---___------_
SRCLK : __-__-__-__-__
Data     -1--2--3--4- <-送られてきた順番を表す
LSBかMSBは実際の回路との兼ね合いで決める. 
LED1の出力をデータの最下位ビットと決めた場合、MSBで入力すればよい. 

*/
byte data = 0B00000000;

void setLED_MsbFirst(){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}

void setLED_LsbFirst(){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode( dataPin, OUTPUT);
  setLED_MsbFirst();//data=0b00000000;なので、全部消灯となる
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  data ++;
  setLED_MsbFirst();
  delay(500);
}
