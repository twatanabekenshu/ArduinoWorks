#include <Arduino_FreeRTOS.h>
#include <event_groups.h>

//イベントグループのためのフラグハンドラ
EventGroupHandle_t eventFlags;

//各タスクのためのフラグ:
const int TASK_RED_LED_FLAG = (1 << 0);
const int TASK_YEL_LED_FLAG = (1 << 1);
const int TASK_BLU_LED_FLAG = (1 << 2);
const int  TASK_BUZZER_FLAG = (1 << 3);

//周波数は適当でOK
const float buzzerFreq = 261.626;


#define RED_LED_PIN  9
#define YEL_LED_PIN  6
#define BLU_LED_PIN  3
#define  BUZZER_PIN 12  

//LEDのタスク実行関数のための構造体
typedef struct{
  int selfFlag; //この関数を実行するときのフラグの場所を表す
  int LED_pin;  //このLEDのピン番号を格納する
  int nextFlag; //次に実行する関数のためにどのフラグを立てるかを格納する
}LED_task_args;

const LED_task_args red_LED = {TASK_RED_LED_FLAG, RED_LED_PIN, TASK_YEL_LED_FLAG};
const LED_task_args yel_LED = {TASK_YEL_LED_FLAG, YEL_LED_PIN, TASK_BLU_LED_FLAG};
const LED_task_args blu_LED = {TASK_BLU_LED_FLAG, BLU_LED_PIN,  TASK_BUZZER_FLAG};

TaskHandle_t    redLED_task_h;
TaskHandle_t yellowLED_task_h;
TaskHandle_t   blueLED_task_h;
TaskHandle_t    buzzer_task_h;

void LED_task(void*);
void buzzer_task(void*);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  eventFlags = xEventGroupCreate();
  if(eventFlags != NULL){
    xTaskCreate(   LED_task,    "RED", 128, &red_LED, 1,    &redLED_task_h);
    xTaskCreate(   LED_task, "YELLOW", 128, &yel_LED, 1, &yellowLED_task_h);
    xTaskCreate(   LED_task,   "BLUE", 128, &blu_LED, 1,   &blueLED_task_h);
    xTaskCreate(buzzer_task, "BUZZER", 128,    NULL, 1,    &buzzer_task_h);

    xEventGroupSetBits(eventFlags, TASK_RED_LED_FLAG);

    vTaskStartScheduler();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

//LEDの点灯方法は統一:今後LEDの点灯方法を個別に指定するなら書き直すのを前提に作成
void LED_task(void* args){
  LED_task_args* params = (LED_task_args*)args;
  pinMode(params->LED_pin, OUTPUT);
  while(1){
    //このLEDのフラグを確認する
    xEventGroupWaitBits(
      eventFlags,         // xEventGroup : イベントグループの識別子
      params->selfFlag,   // uxBitsToWaitFor : 確認するビット
      pdTRUE,             // xClearOnExit : 完了後に自動更新するかどうか
      pdTRUE,             // xWaitForAllBits : 複数ビットを確認する場合、TRUEならOR論理、FALSEならAND論理になる. 1つのビットだけ確認するならどちらでもよい
      portMAX_DELAY       // 待ち時間の設定(フラグが立つまで無制限に待つ)
    );
    //点灯3秒
    digitalWrite(params->LED_pin, HIGH);
    vTaskDelay(3000/portTICK_PERIOD_MS);
    //消灯1秒
    digitalWrite(params->LED_pin,  LOW);
    vTaskDelay(1000/portTICK_PERIOD_MS);

    for(int i = 0; i < 5; i ++){
      //点灯3秒
      digitalWrite(params->LED_pin, HIGH);
      vTaskDelay(250/portTICK_PERIOD_MS);
      //消灯1秒
      digitalWrite(params->LED_pin,  LOW);
      vTaskDelay(250/portTICK_PERIOD_MS);
    }

    //次のLEDやブザーのフラグを立てる
    xEventGroupSetBits(eventFlags, params->nextFlag);
  }
}

void buzzer_task(void* args){
  int countLoop = 0;
  while(1){
    xEventGroupWaitBits(eventFlags, TASK_BUZZER_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
    countLoop ++;
    for(int i = 0; i < countLoop; i ++){
      tone(BUZZER_PIN, buzzerFreq);
      vTaskDelay(3000/portTICK_PERIOD_MS);
      noTone(BUZZER_PIN);
      vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    Serial.print(countLoop);
    Serial.println("周目が終わりました");
    xEventGroupSetBits(eventFlags, TASK_RED_LED_FLAG);
  }
}
