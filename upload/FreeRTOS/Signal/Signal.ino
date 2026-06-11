#include <Arduino_FreeRTOS.h>

/*
鉄道信号の模擬
LEDは赤1黄色3緑1を使う。

接続:
 2:ボタンスイッチ
 4:緑
 7:黄1
 8:赤
12:黄2
13:黄3

各状態に対し定められている遷移時間が経過するごとに赤→黄色2個(1,3)→黄色1個(1)→黄(2)と緑1個ずつ→緑と変化する。
ただし、ボタンが押されると現在の信号にかかわらず赤になり、黄色2個->...のように変化する。
ボタンを押す→列車の通過を再現(本来は赤でさらに押されることはない)
*/

const int BTN_PIN =  2;

const int GRN_PIN =  4;
const int YE1_PIN =  7;
const int RED_PIN =  8;
const int YE2_PIN = 12;
const int YE3_PIN = 13;

typedef enum{
  PROCEED = 0,
  PRE_CAUTION = 1,
  CAUTION = 2,
  RESTRICTING = 3,
  STOP = 4
}ASPECT;

//各信号現示の待ち時間(TICK数)
const TickType_t WAIT_TICK_ARRAY[5] = {
  portMAX_DELAY, 
  5000 / portTICK_PERIOD_MS, 
  5000 / portTICK_PERIOD_MS, 
  5000 / portTICK_PERIOD_MS, 
  5000 / portTICK_PERIOD_MS
};

ASPECT signal_aspect;
TaskHandle_t signalLight_task;
TaskHandle_t signalTimer_task;
TaskHandle_t btnChk_task;

void signalLightControl(void* args){
  pinMode(GRN_PIN, OUTPUT);
  pinMode(YE1_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(YE2_PIN, OUTPUT);
  pinMode(YE3_PIN, OUTPUT);
  while(1){
    //信号現示が変わるまでは何もしなくていいので通知を待つ
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    switch(signal_aspect){
      case PROCEED : 
        digitalWrite(GRN_PIN, HIGH);
        digitalWrite(YE1_PIN,  LOW);
        digitalWrite(RED_PIN,  LOW);
        digitalWrite(YE2_PIN,  LOW);
        digitalWrite(YE3_PIN,  LOW);
        break;
      case PRE_CAUTION : 
        digitalWrite(GRN_PIN, HIGH);
        digitalWrite(YE1_PIN,  LOW);
        digitalWrite(RED_PIN,  LOW);
        digitalWrite(YE2_PIN, HIGH);
        digitalWrite(YE3_PIN,  LOW);
        break;
      case CAUTION : 
        digitalWrite(GRN_PIN,  LOW);
        digitalWrite(YE1_PIN, HIGH);
        digitalWrite(RED_PIN,  LOW);
        digitalWrite(YE2_PIN,  LOW);
        digitalWrite(YE3_PIN,  LOW);
        break;
      case RESTRICTING : 
        digitalWrite(GRN_PIN,  LOW);
        digitalWrite(YE1_PIN, HIGH);
        digitalWrite(RED_PIN,  LOW);
        digitalWrite(YE2_PIN,  LOW);
        digitalWrite(YE3_PIN, HIGH);
        break;
      case STOP : 
      default : 
        digitalWrite(GRN_PIN,  LOW);
        digitalWrite(YE1_PIN,  LOW);
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(YE2_PIN,  LOW);
        digitalWrite(YE3_PIN,  LOW);
        break;
    }
  }
}

void signalStateTimer(void* args){
  while(1){
    TickType_t waitTick = WAIT_TICK_ARRAY[signal_aspect];
    //タイムアウトした場合(待ってもボタンを押された通知がなかった場合),時間に従って通り現示を進める
    if(ulTaskNotifyTake(pdTRUE, waitTick) == 0){
      switch(signal_aspect){
        case PROCEED : 
          break;
        case PRE_CAUTION : 
          signal_aspect = PROCEED;
          xTaskNotifyGive(signalLight_task);
          break;
        case CAUTION : 
          signal_aspect = PRE_CAUTION;
          xTaskNotifyGive(signalLight_task);
          break;
        case RESTRICTING : 
          signal_aspect = CAUTION;
          xTaskNotifyGive(signalLight_task);
          break;
        case STOP : 
          signal_aspect = RESTRICTING;
          xTaskNotifyGive(signalLight_task);
          break;
        default : 
          signal_aspect = STOP;
          xTaskNotifyGive(signalLight_task);
          break;
      }
    }else{
      signal_aspect = STOP;
      xTaskNotifyGive(signalLight_task);
    }
  }
}

void btnChk(void* args){
  pinMode(BTN_PIN, INPUT_PULLUP);
  bool    last_btn_state = HIGH;
  bool current_btn_state = HIGH;
  //ボタン状態を監視する
  while(1){
    //前回は押されていなかったが、今回は押されているとき
    current_btn_state = digitalRead(BTN_PIN);
    if(current_btn_state == LOW && last_btn_state == HIGH){
      vTaskDelay(2);
      if(digitalRead(BTN_PIN) == LOW){
        xTaskNotifyGive(signalTimer_task);
      }
    }
    last_btn_state = current_btn_state;
  }
}

void setup() {
  // put your setup code here, to run once:
  signal_aspect = STOP;
  
  xTaskCreate(signalLightControl, "SIGNAL_LIGHT", 128, NULL, 1, &signalLight_task);
  xTaskNotifyGive(signalLight_task);
  xTaskCreate(  signalStateTimer, "SIGNAL_STATE", 128, NULL, 1, &signalTimer_task);
  xTaskCreate(            btnChk,      "BTN_CHK", 128, NULL, 1, &btnChk_task);

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
