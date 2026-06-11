/*
ピンへの入力を監視する. 
今回割り込みを使わない. 
pin2:スイッチ
pin4:LED1
pin7:LED2
最初は両方のLEDが消灯している。
スイッチを押した回数は0,1,2,...と数える。
スイッチが押されるごとに回数を増やす。
回数を3で割った余りが0ならば両方のLEDを消灯する。
回数を3で割った余りが1ならばpin4のLEDのみ点灯する。
回数を3で割った余りが2ならばpin4と7のLEDが両方点灯する。
*/

const int LED_1_pin = 4;
const int LED_2_pin = 7;
const int btn_pin   = 2;

//監視周期を変える:短すぎるとチャタリングに弱くなり、長すぎると検出ミスを起こす
const int MONITOR_INTERVAL = 500;

bool is_pushed = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_1_pin, OUTPUT);
  pinMode(LED_2_pin, OUTPUT);
  pinMode(  btn_pin, INPUT_PULLUP);
  digitalWrite(LED_1_pin, LOW);
  digitalWrite(LED_2_pin, LOW);
}

int push_count = 0;

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(btn_pin) == LOW){
    if(!is_pushed){
      is_pushed = true;
      push_count ++;
    }
  }else{
    is_pushed = false;
  }

  switch(push_count % 3){
    case 0:
      digitalWrite(LED_1_pin, LOW);
      digitalWrite(LED_2_pin, LOW);
      break;
    case 1:
      digitalWrite(LED_1_pin, HIGH);
      digitalWrite(LED_2_pin, LOW);
      break;
    case 2:
      digitalWrite(LED_1_pin, HIGH);
      digitalWrite(LED_2_pin, HIGH);
      break;
    default:
      digitalWrite(LED_1_pin, LOW);
      digitalWrite(LED_2_pin, LOW);
      break;
  }
  delay(MONITOR_INTERVAL);
}
