/*
ボタン:タクトスイッチ(並列接続)
LED:7個(追加機能実装時は9個)
車道信号機用:赤・黄・緑 各1個
歩道信号機用:赤・緑 各2個(赤、緑どうしでそれぞれ並列につないでおき、1つのピンで赤も緑も制御する)
緑LEDの色のほうが実際の青信号に近い色であるような気がするので、LEDは緑にしてある

下記のLEDは追加機能とする
「お待ちください」表示のLED(白)2個(並列につなぎ、1つにピンで制御する)
押しボタンが押されてから歩行者信号が青になるまでの間点灯するLED
その他の時間は消灯している

ピン数
ボタン:1
LED:6

信号機の状態
車道:赤・黄・青
歩道:赤・青・青点滅
ただし、9通りの状態になるわけではない点に注意。例えば歩道も車道も両方青の状態はあり得ない。
これを踏まえると次の5通りの状態となる。

(車道, 歩道)={(青, 赤), (黄, 赤), (赤, 赤), (赤, 青), (赤, 青点滅)};
状態遷移はこのようになる。
(青, 赤)->(黄, 赤)->(赤, 赤)->(赤, 青)->(赤, 青点滅)->(赤, 赤)->(青, 赤)->...
遷移時間はそれぞれ一定の時間とする。
最初はそれぞれ10, 3, 3, 5, 4, 3秒とする。
ただし、(青, 赤)以外の状態でボタンが押された場合はボタン入力を無視する。

(赤, 赤)の場合は車道が赤になったのか歩行者信号が赤になったのかで場合分けが必要
LED制御と状態遷移は分けたほうがよさそう。
*/

#include <Arduino_FreeRTOS.h>
//2つの信号機の状態の組み合わせを定義する
typedef enum{
  CAR_BLUE        = 0,  //車道:青・歩道:赤
  CAR_YELLOW      = 1,  //車道:黄・歩道:赤,
  ALL_RED_PRE_PED = 2,  //車道:赤・歩道:赤(次に歩道が青になる)
  PED_BLUE        = 3,  //車道:赤・歩道:青
  PED_BLINK       = 4,  //車道:赤・歩道:青点滅
  ALL_RED_PRE_CAR = 5   //車道:赤・歩道:赤(次に車道が青になる)
}ASPECTS;

ASPECTS state;
int period_times[6] = { 
     portMAX_DELAY,             //車道が青の時間を表す(ボタンが押されるまでずっと青とする)
     3000 / portTICK_PERIOD_MS, //車道が黄の時間を表す
     3000 / portTICK_PERIOD_MS, //歩道が青になる前に、両方赤になっている時間を表す
     5000 / portTICK_PERIOD_MS, //歩道が青の時間を表す
     4000 / portTICK_PERIOD_MS, //歩道が青点滅の時間を表す
     3000 / portTICK_PERIOD_MS, //車道が青になる前に両方赤になっている時間を表す
  };

//歩行者の青点滅の回数を表す
int blink_max_times = 10;

const int PEDESTRIAN_BTN_PIN = 2;
const int PEDESTRIAN_RED_PIN = 4;
const int PEDESTRIAN_BLU_PIN = 7;

const int PEDESTRIAN_WAIT_LED= 5;

const int CAR_RED_PIN =  8;
const int CAR_YEL_PIN = 12;
const int CAR_BLU_PIN = 13;

TaskHandle_t  button_check_task;
TaskHandle_t   LED_control_task;
TaskHandle_t state_control_task;

void button_check_func(void* args){
  //車道が青の場合に限り、ボタンの状態チェックを行う。押された瞬間HIGH->LOW->LOW
  bool    last_btn_state = HIGH;
  bool current_btn_state = HIGH;
  while(1){
    if(state == CAR_BLUE){
      current_btn_state = digitalRead(PEDESTRIAN_BTN_PIN);
      if(current_btn_state == LOW && last_btn_state == HIGH){
        vTaskDelay(1);
        if(digitalRead(PEDESTRIAN_BTN_PIN) == LOW){
          digitalWrite(PEDESTRIAN_WAIT_LED, HIGH);
          vTaskDelay(2000/portTICK_PERIOD_MS);
          xTaskNotifyGive(state_control_task);
        }
      }
      last_btn_state = current_btn_state;
    }else{
      //青以外ではlast_btn_stateをHIGHにリセットしておく
      //last_btn_state = HIGH;
    }
    vTaskDelay(1);
  }
}

void state_control_func(void* args){
  //点滅回数を記録しておくための変数
  int blink_cnt = 0;
  int delay_ticks = 0;
  while(1){
    delay_ticks = period_times[state];
    //毎回計算するのは無駄なので設定が変わった、状態が変わったときだけ計算するロジックのほうがよさそう
    if(state == PED_BLINK){
      delay_ticks /= (2*blink_max_times);
    }
    //待ち時間を完了してタイムアウトになった場合=TaskNotifyをちゃんと受け取れなかった場合
    if(ulTaskNotifyTake(pdTRUE, delay_ticks) == 0){
      switch(state){
        case CAR_BLUE :
          state = CAR_YELLOW;
          xTaskNotifyGive(LED_control_task);
          break;
        case CAR_YELLOW :
          state = ALL_RED_PRE_PED;
          xTaskNotifyGive(LED_control_task);
          break;
        case ALL_RED_PRE_PED :
          state = PED_BLUE;
          xTaskNotifyGive(LED_control_task);
          break;
        case PED_BLUE :
          state = PED_BLINK;
          xTaskNotifyGive(LED_control_task);
          break;
        case ALL_RED_PRE_CAR :
          state = CAR_BLUE;
          xTaskNotifyGive(LED_control_task);
          break;
        case PED_BLINK :
          blink_cnt ++;
          if(blink_cnt == 2*blink_max_times){
            state = ALL_RED_PRE_CAR;
            blink_cnt = 0;
          }
          xTaskNotifyGive(LED_control_task);
          break;
        default :
          xTaskNotifyGive(LED_control_task);
          break;
      }
    }else{
      //タイムアウトにならなかった場合→青ならば黄色に変える
      if(state == CAR_BLUE){
        state = CAR_YELLOW;
        xTaskNotifyGive(LED_control_task);
      }
    }
  }
}

void LED_control_func(void* args){
  while(1){
    //点灯するLEDが変わったときだけ動けばいいので分離
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    switch(state){
      case CAR_BLUE :
        //車道:青・歩道:赤
        digitalWrite(PEDESTRIAN_RED_PIN, HIGH);
        digitalWrite(PEDESTRIAN_BLU_PIN, LOW);
        digitalWrite(CAR_RED_PIN, LOW);
        digitalWrite(CAR_YEL_PIN, LOW);
        digitalWrite(CAR_BLU_PIN, HIGH);
        break;
      case CAR_YELLOW :
        //車道:黄・歩道:赤
        digitalWrite(PEDESTRIAN_RED_PIN, HIGH);
        digitalWrite(PEDESTRIAN_BLU_PIN, LOW);
        digitalWrite(CAR_RED_PIN, LOW);
        digitalWrite(CAR_YEL_PIN, HIGH);
        digitalWrite(CAR_BLU_PIN, LOW);
        break;
      case ALL_RED_PRE_PED :
        //車道:赤・歩道:赤(次に歩道が青になる)
        digitalWrite(PEDESTRIAN_RED_PIN, HIGH);
        digitalWrite(PEDESTRIAN_BLU_PIN, LOW);
        digitalWrite(CAR_RED_PIN, HIGH);
        digitalWrite(CAR_YEL_PIN, LOW);
        digitalWrite(CAR_BLU_PIN, LOW);
        break;
      case PED_BLUE :
        //車道:赤・歩道:青
        digitalWrite(PEDESTRIAN_WAIT_LED, LOW);
        digitalWrite(PEDESTRIAN_RED_PIN, LOW);
        digitalWrite(PEDESTRIAN_BLU_PIN, HIGH);
        digitalWrite(CAR_RED_PIN, HIGH);
        digitalWrite(CAR_YEL_PIN, LOW);
        digitalWrite(CAR_BLU_PIN, LOW);
        break;
      case PED_BLINK :
        //車道:赤・歩道:青点滅
        digitalWrite(PEDESTRIAN_RED_PIN, LOW);
        digitalWrite(PEDESTRIAN_BLU_PIN, !digitalRead(PEDESTRIAN_BLU_PIN));//ピンの状態を見て反転させることで、点滅するようになる
        digitalWrite(CAR_RED_PIN, HIGH);
        digitalWrite(CAR_YEL_PIN, LOW);
        digitalWrite(CAR_BLU_PIN, LOW);
        break;
      case ALL_RED_PRE_CAR :
        //車道:赤・歩道:赤(次に車道が青になる)
        digitalWrite(PEDESTRIAN_RED_PIN, HIGH);
        digitalWrite(PEDESTRIAN_BLU_PIN, LOW);
        digitalWrite(CAR_RED_PIN, HIGH);
        digitalWrite(CAR_YEL_PIN, LOW);
        digitalWrite(CAR_BLU_PIN, LOW);
        break;
      default :
        //未定義の値が来た場合 : 車道:赤・歩道:
        digitalWrite(PEDESTRIAN_RED_PIN, HIGH);
        digitalWrite(PEDESTRIAN_BLU_PIN, LOW);
        digitalWrite(CAR_RED_PIN, HIGH);
        digitalWrite(CAR_YEL_PIN, LOW);
        digitalWrite(CAR_BLU_PIN, LOW);
        break;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(PEDESTRIAN_WAIT_LED,OUTPUT);
  pinMode(PEDESTRIAN_BTN_PIN, INPUT_PULLUP);
  pinMode(PEDESTRIAN_RED_PIN, OUTPUT);
  pinMode(PEDESTRIAN_BLU_PIN, OUTPUT);
  pinMode(CAR_RED_PIN, OUTPUT);
  pinMode(CAR_YEL_PIN, OUTPUT);
  pinMode(CAR_BLU_PIN, OUTPUT);
  
  state = CAR_BLUE;

  xTaskCreate(LED_control_func, "LED_CTRL", 128, NULL, 1, &LED_control_task);
  xTaskCreate(state_control_func, "STATE_CTRL", 128, NULL, 1, &state_control_task);
  xTaskCreate(button_check_func, "BTN_CHK", 128, NULL, 1, &button_check_task);
  xTaskNotifyGive(LED_control_task);
  vTaskStartScheduler();
}

void loop() {}
