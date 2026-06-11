#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

//2つの信号機の状態の組み合わせを定義する
typedef enum{
  CAR_BLUE            = 0,  //車道:青・歩道:赤
  CAR_BLUE_BTN_PUSHED = 1,  //車道:青・歩道:赤, ボタンが押されている状態
  CAR_YELLOW          = 2,  //車道:黄・歩道:赤,
  ALL_RED_PRE_PED     = 3,  //車道:赤・歩道:赤(次に歩道が青になる)
  PED_BLUE            = 4,  //車道:赤・歩道:青
  PED_BLINK           = 5,  //車道:赤・歩道:青点滅
  ALL_RED_PRE_CAR     = 6,  //車道:赤・歩道:赤(次に車道が青になる)
  ALL_RED_ON_ERROR    = 7,  //異常発生時:車道も歩道も赤になる、この状態からはリセットする以外で復帰できない
}ASPECTS;


const int CAR_BLUE_BTN_PUSHED_ms = 2000;
const int CAR_YELLOW_ms = 3000;
const int ALL_RED_PRE_PED_ms = 3000;
const int PED_BLINK_ms = 200;
const int ALL_RED_PRE_CAR_ms = 3000;

volatile int PED_BLUE_ms = 5000;

const int PEDESTRIAN_BTN_PIN = 2;//PD2
const int PEDESTRIAN_RED_PIN = 4;//PD4
const int PEDESTRIAN_BLU_PIN = 7;//PD7

const int PEDESTRIAN_WAIT_LED= 5;//PD5

const int CAR_RED_PIN =  8;//PB0
const int CAR_YEL_PIN = 12;//PB4
const int CAR_BLU_PIN = 13;//PB5

TaskHandle_t  serial_check_task;
TaskHandle_t  button_check_task;
TaskHandle_t   LED_control_task;
TaskHandle_t state_control_task;
TaskHandle_t     beepSound_task;

SemaphoreHandle_t state_mutex;
SemaphoreHandle_t periodTime_mutex;

SemaphoreHandle_t 

//タスク通知でも実現できるが、可読性のためにイベントフラグにする
EventGroupHandle_t sound_flag_group;
const int SOUND_START = (1 << 0);
const int SOUND_STOP  = (1 << 1);

//外部からのアクセスを禁止する→本当はgetterも欲しい
static ASPECTS state;
//外部アクセス禁止の代わりにsetterを作成する
void setState(ASPECTS newState){
  xSemaphoreTake(state_mutex, portMAX_DELAY);
  state = newState;
  xSemaphoreGive(state_mutex);
}

void button_check_func(void* args){
  //安定化のためシリアル通信を先に行わせる
  vTaskDelay(pdMS_TO_TICKS(500));
  //車道が青の場合に限り、ボタンの状態チェックを行う。押された瞬間(HIGH->LOW->LOW)を検出する
  bool    last_btn_state = HIGH;
  bool current_btn_state = HIGH;
  unsigned long low_start_time = 0;
  bool is_pressing = false;
  
  while(1){
    if(state == CAR_BLUE){
      /*
      HIGHからLOWに変わったところを検出
      →一定時間後もまだLOWになっているかを確認する
      →押されたままならLOWになっていると判断
      is_pressingは「押されている可能性がある」というフラグ
      */
      bool current_btn_state = digitalRead(PEDESTRIAN_BTN_PIN);
      
      // ボタンがHIGHからLOWに変わった瞬間(押し始め)
      if(current_btn_state == LOW && last_btn_state == HIGH){
        low_start_time = millis(); // 押し始めた時間を記録
        is_pressing = true;
      }
      
      // ボタンがLOWのままキープされているかチェック
      if(is_pressing && current_btn_state == LOW){
        // チャタリングなどであればLOWのままキープとはならないはず、これまでの実験から20msくらいで十分?
        if(millis() - low_start_time >= 20){ 
          setState(CAR_BLUE_BTN_PUSHED);
          xTaskNotifyGive(state_control_task);
          is_pressing = false;
        }
      }
      
      // ボタンが途中で離されたらリセット
      if(current_btn_state == HIGH){
        is_pressing = false;
      }
      
      last_btn_state = current_btn_state;
    } else {
      is_pressing = false; // CAR_BLUE 以外の状態の時はリセット
    }
    // ここで他のタスクに1TICK分、CPUを譲る
    vTaskDelay(1);
  }
}

//歩行者の青点滅の回数を表す。ただし、点灯・消灯をそれぞれ1回と数える
const int BLINK_COUNT_MAX = 20;

void state_control_func(void* args){
  while(1){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    //この先は「LED_controlTaskに点灯しているLEDを変えてもらった後、一定時間待つこと」を繰り返す。
    xTaskNotifyGive(LED_control_task);
    vTaskDelay(pdMS_TO_TICKS(CAR_BLUE_BTN_PUSHED_ms));
    
    setState(CAR_YELLOW);
    xTaskNotifyGive(LED_control_task);
    vTaskDelay(pdMS_TO_TICKS(CAR_YELLOW_ms));

    setState(ALL_RED_PRE_PED);
    xTaskNotifyGive(LED_control_task);
    vTaskDelay(pdMS_TO_TICKS(ALL_RED_PRE_PED_ms));

    setState(PED_BLUE);
    xEventGroupSetBits(sound_flag_group, SOUND_START);
    xSemaphoreTake(periodTime_mutex, portMAX_DELAY);
    int local_ped_blue_ms = PED_BLUE_ms;
    xSemaphoreGive(periodTime_mutex);
    xTaskNotifyGive(LED_control_task);
    vTaskDelay(pdMS_TO_TICKS(local_ped_blue_ms));

    setState(PED_BLINK);
    xEventGroupSetBits(sound_flag_group, SOUND_STOP);
    for(int i = 0; i < BLINK_COUNT_MAX; i ++){
      xTaskNotifyGive(LED_control_task);
      vTaskDelay(pdMS_TO_TICKS(PED_BLINK_ms));
    }

    setState(ALL_RED_PRE_CAR);
    xTaskNotifyGive(LED_control_task);
    vTaskDelay(pdMS_TO_TICKS(ALL_RED_PRE_CAR_ms));

    setState(CAR_BLUE);
    xTaskNotifyGive(LED_control_task);
  }
}

void LED_control_func(void* args){
  while(1){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(state_mutex, portMAX_DELAY);
    switch(state){
      case CAR_BLUE :
        //車道:青・歩道:赤/歩行者ボタン待ちLED消灯
        PORTD |= (1 << PEDESTRIAN_RED_PIN);
          PORTD &= ~((1 << PEDESTRIAN_WAIT_LED) | (1 << PEDESTRIAN_BLU_PIN));
        PORTB |= (1 << (CAR_BLU_PIN-8));
        PORTB &= ~((1 << (CAR_RED_PIN-8))|(1 << (CAR_YEL_PIN-8)));
        break;
      case CAR_BLUE_BTN_PUSHED :
        //車道:青・歩道:赤/歩行者ボタン待ちLED点灯
        PORTD |= (1 << PEDESTRIAN_WAIT_LED) | (1 << PEDESTRIAN_RED_PIN);
        PORTD &= ~ (1 << PEDESTRIAN_BLU_PIN);
        PORTB |= (1 << (CAR_BLU_PIN-8));
        PORTB &= ~((1 << (CAR_RED_PIN-8))|(1 << (CAR_YEL_PIN-8)));
        break;
      case CAR_YELLOW :
        //車道:黄・歩道:赤/歩行者ボタン待ちLED点灯
        PORTD |= (1 << PEDESTRIAN_WAIT_LED) | (1 << PEDESTRIAN_RED_PIN);
        PORTD &= ~ (1 << PEDESTRIAN_BLU_PIN);
        PORTB |= (1 << (CAR_YEL_PIN-8));
        PORTB &= ~((1 << (CAR_RED_PIN-8))|(1 << (CAR_BLU_PIN-8)));
        break;
      case ALL_RED_PRE_PED :
        //車道:赤・歩道:赤(次に歩道が青になる)/歩行者待ちLED点灯
        PORTD |= (1 << PEDESTRIAN_WAIT_LED) | (1 << PEDESTRIAN_RED_PIN);
        PORTD &= ~ (1 << PEDESTRIAN_BLU_PIN);
        PORTB |= (1 << (CAR_RED_PIN-8));
        PORTB &= ~((1 << (CAR_YEL_PIN-8))|(1 << (CAR_BLU_PIN-8)));
        break;
      case PED_BLUE :
        //車道:赤・歩道:青/歩行者待ちLED消灯
        PORTD |= (1 << PEDESTRIAN_BLU_PIN);
        PORTD &= ~((1 << PEDESTRIAN_WAIT_LED) | (1 << PEDESTRIAN_RED_PIN));
        PORTB |= (1 << (CAR_RED_PIN-8));
        PORTB &= ~((1 << (CAR_YEL_PIN-8))|(1 << (CAR_BLU_PIN-8)));
        break;
      case PED_BLINK :
        //車道:赤・歩道:青点滅
        PORTD ^= (1 << PEDESTRIAN_BLU_PIN);
        PORTD &= ~((1 << PEDESTRIAN_WAIT_LED) | (1 << PEDESTRIAN_RED_PIN));
        PORTB |= (1 << (CAR_RED_PIN-8));
        PORTB &= ~((1 << (CAR_YEL_PIN-8))|(1 << (CAR_BLU_PIN-8)));
        break;
      case ALL_RED_PRE_CAR :
        //車道:赤・歩道:赤(次に車道が青になる)
        PORTD |= (1 << PEDESTRIAN_RED_PIN);
        PORTD &= ~ ((1 << PEDESTRIAN_BLU_PIN) | (1 << PEDESTRIAN_WAIT_LED));
        PORTB |= (1 << (CAR_RED_PIN-8));
        PORTB &= ~((1 << (CAR_YEL_PIN-8))|(1 << (CAR_BLU_PIN-8)));
        break;
      case ALL_RED_ON_ERROR : 
      default :
        //未定義の値が来た場合 : 車道:赤・歩道:赤
        PORTD |= (1 << PEDESTRIAN_RED_PIN);
        PORTD &= ~ ((1 << PEDESTRIAN_BLU_PIN) | (1 << PEDESTRIAN_WAIT_LED));
        PORTB |= (1 << (CAR_RED_PIN-8));
        PORTB &= ~((1 << (CAR_YEL_PIN-8))|(1 << (CAR_BLU_PIN-8)));
        break;
    }
    xSemaphoreGive(state_mutex);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  DDRD &= ~(1 << PEDESTRIAN_BTN_PIN);
  PORTD|= (1 << PEDESTRIAN_BTN_PIN);
  DDRD |= (1 << PEDESTRIAN_RED_PIN) | (1 << PEDESTRIAN_BLU_PIN) | (1 << PEDESTRIAN_WAIT_LED);
  //「ピン番号-8」はピン番号をポートに対応させるための処理
  DDRB |= (1 << (CAR_RED_PIN-8)) | (1 << (CAR_YEL_PIN-8)) | (1 << (CAR_BLU_PIN-8));
  state = CAR_BLUE;
  
       state_mutex = xSemaphoreCreateMutex();
  periodTime_mutex = xSemaphoreCreateMutex();

  sound_flag_group = xEventGroupCreate();

  xTaskCreate( serial_check_func, "serial_check", 128, NULL, 1, &serial_check_task);
  xTaskCreate(state_control_func,   "STATE_CTRL", 128, NULL, 2, &state_control_task);
  xTaskCreate(  LED_control_func,     "LED_CTRL", 128, NULL, 2, &LED_control_task);
  xTaskCreate(    beepSound_func,   "BEEP_SOUND", 128, NULL, 2, &beepSound_task);
  xTaskCreate( button_check_func,      "BTN_CHK", 128, NULL, 3, &button_check_task);
  xTaskNotifyGive(LED_control_task);
  vTaskStartScheduler();
}

void loop() {}

void serial_check_func(void* args){
  /*F("文字列"):文字列をスタックメモリではなく、フラッシュメモリに取るためのマクロ->スタックオーバーフローの防止のために入れておく*/
  Serial.println(F("横断時間設定"));
  Serial.print(F("現在の設定 : "));
  Serial.print(PED_BLUE_ms);
  Serial.println(F("ミリ秒"));

  unsigned long inputedTime = 0;
  while(1){
    if(Serial.available() > 0){
      if(state != CAR_BLUE){
        Serial.println(F("通常状態のときに入力してください"));
        while(Serial.available() > 0){Serial.read();}
        continue;
      }
      inputedTime = Serial.parseInt();
      
      if(inputedTime < 1000 || inputedTime > 20000){
        Serial.println(F("1000から20000の範囲の整数で指定してください"));
      }else{
          xSemaphoreTake(periodTime_mutex, portMAX_DELAY);
          PED_BLUE_ms = inputedTime;
          xSemaphoreGive(periodTime_mutex);

          Serial.print(inputedTime);
          Serial.println(F("ミリ秒に時間を変更しました"));
      }
    }
    //約80ms
    vTaskDelay(5);
  }
}

#define BUZZER_PIN 6

void beepSound_func(void* args){
  unsigned int soundData[3][2] = {{1600,300},{1280,300},{0, 300}};
  while(1){
    xEventGroupClearBits(sound_flag_group, SOUND_START | SOUND_STOP);
    xEventGroupWaitBits(sound_flag_group, SOUND_START, pdTRUE, pdTRUE, portMAX_DELAY);
    
    for(int i = 0; ; i ++){
      if(i >= 3){i = 0;}
      if(soundData[i][0] == 0){
        noTone(BUZZER_PIN);
      }else{
        tone(BUZZER_PIN, soundData[i][0]);
      }

      EventBits_t bits = xEventGroupWaitBits(sound_flag_group, SOUND_STOP, pdTRUE, pdTRUE, pdMS_TO_TICKS(soundData[i][1]));
      if((bits & SOUND_STOP) != 0){
        noTone(BUZZER_PIN);
        break;
      }
    }
  }
}
