/*
暗くなると点灯するシステム
*/

#include <avr/wdt.h>
#include <MsTimer2.h>

/*
このシステムの状態を表す列挙型:
*/
typedef enum{
  OFF,    //消灯
  TR_ON,  //点灯移行
  ON,     //点灯
  TR_OFF  //消灯以降
}STATUS;

//要調整:CdSが暗いと判断する基準値
#define CDS_THRESHOLD 400

typedef enum{
  DARK,
  BRIGHT
}AROUND;

STATUS status;

unsigned long millis_count;
unsigned long millis_now;

void checkCdS(){
  if(analogRead(A0) > CDS_THRESHOLD){
    //周りが暗いとき:
    //現在の状態がOFFの場合はTR_ONに移行する
    if(status == OFF){
      status = TR_ON;
      millis_count = millis();
    }else if(status == TR_OFF){
      //現在の状態がTR_OFFの場合はONに移行する
      status = ON;
      millis_count = millis();
    }
  }else{
    //周りが明るいとき:
    //現在の状態がONの場合はTR_OFFに移行する
    if(status == ON){
      status = TR_OFF;
      millis_count = millis();
    }else if(status == TR_ON){
      //現在の状態がTR_ONの場合はOFFに移行する
      status = OFF;
      millis_count = millis();
    }
  }
}

/*
シリアル通信を読み込み、「e」と入力されたらdelay(1000)を10回行う
エラーでloop()処理が止まったことを模擬する
このエラーの模擬はwdtでリセットされる
*/
void emuError(){
  if(Serial.available() > 0){
    char c = Serial.read();
    delay(100);
    while(Serial.available() > 0){
      Serial.read();
    }
    if(c == 'e'){
      Serial.println("An error has occured");
      for(int i = 0; i < 10; i ++){
        delay(1000);
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("launched");
  MsTimer2::set(100, checkCdS);
  MsTimer2::start();

  wdt_enable(WDTO_2S);
  DDRD  |=   0B00000100;
  PORTD &= ~(0B00000100);
  status = OFF;
}

void loop() {
  // put your main code here, to run repeatedly:
  emuError();
  millis_now = millis();
  switch(status){
    case    OFF : 
      //消灯する
      PORTD &= ~(0B00000100);
      break;
    case  TR_ON : 
      if(millis_now - millis_count >= 3000){
        status = ON;
      }
      break;
    case     ON : 
      //点灯する
      PORTD |= (0B00000100);
      break;
    case TR_OFF : 
      if(millis_now - millis_count >= 3000){
        status = OFF;
      }
      break;
    default : 
      break;
  }
  wdt_reset();
}
