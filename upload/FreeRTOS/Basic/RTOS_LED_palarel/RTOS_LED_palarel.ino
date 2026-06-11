#include <Arduino_FreeRTOS.h> 
#include <semphr.h>

typedef enum{
   OFF, //完全に消灯している状態
    UP, //明るくなっている状態
  DOWN, //暗くなっている状態
    ON  //完全に点灯している状態
}LEDState;

typedef struct{
  int pinNo;
  int brightness;
  LEDState status;
  SemaphoreHandle_t LED_status_Mutex;
}LED;

void LED_task(void* args, int* interval_ms);
void serialReadTask(void* args);

LED red_LED = { 9, 0, OFF};
LED blu_LED = {10, 0, OFF};

TaskHandle_t task_red_LED;
TaskHandle_t task_blu_LED;
TaskHandle_t task_serialRead;

/*
arg はvoid*型とし、
構造体LEDの型で接続されたLEDのピン番号と状態を指定しているものとする*/
void LED_task(void* args){
  LED* targetLED = (LED*)args;
  pinMode(targetLED->pinNo, OUTPUT);
  TickType_t previousWakeTime = xTaskGetTickCount();
  int interval_ms = 10;
  while(1){
    if(xSemaphoreTake(targetLED->LED_status_Mutex, pdMS_TO_TICKS(10))==pdFALSE){
      Serial.print("taking mutex failed");
      continue;
    }
    LED_control(targetLED, &interval_ms);
    xSemaphoreGive(targetLED->LED_status_Mutex);
    vTaskDelayUntil(&previousWakeTime, interval_ms/portTICK_PERIOD_MS);
  }
}

/*LEDのピン出力を変更し、さらにinterval_msに次のタイミングまでの時間を書き込む*/
void LED_control(LED* targetLED, int* interval_ms){
  switch(targetLED->status){
    case   ON : 
      digitalWrite(targetLED->pinNo, HIGH);
      *interval_ms = 10;
      break;
    case  OFF : 
      digitalWrite(targetLED->pinNo, LOW);
      *interval_ms = 10;
      break;
    case   UP :
      if(targetLED->brightness < 255){
        targetLED->brightness ++;
      }else{
        targetLED->status = ON;
      }
      *interval_ms = 50;
      analogWrite(targetLED->pinNo, targetLED->brightness);
      break;
    case DOWN : 
      if(targetLED->brightness > 0){
        targetLED->brightness --;
      }else{
        targetLED->status = OFF;
      }
      *interval_ms = 20;
      analogWrite(targetLED->pinNo, targetLED->brightness);
      break;
  }
}

void changeLEDstatus(LED* targetLED){
  /*
  セマフォを待つ. 最大1のセマフォなのでミューテックス(排他制御)になっている
  待つ対象, 待つ時間(tick単位)pdMS_TO_TICKSでミリ秒からtickに変換している
  */
  if(xSemaphoreTake(targetLED->LED_status_Mutex, pdMS_TO_TICKS(10))==pdFALSE){
    Serial.print("Fail taking mutex on pin : ");
    Serial.println(targetLED->pinNo);
    return;
  }
  //LEDの状態がONかUPのときはDOWNへ、OFFかDOWNのときはUPに状態を変える
  switch(targetLED->status){
    case ON:
    case UP:
      targetLED->status = DOWN;
      break;
    case OFF:
    case DOWN:
      targetLED->status = UP;
      break;
  }
  xSemaphoreGive(targetLED->LED_status_Mutex);
}

void serialReadTask(void* args){
  while(1){
    if(Serial.available() > 0){
      switch(Serial.read()){
        case 'r' : 
          Serial.println('r');
          changeLEDstatus(&red_LED);
          break;
        case 'b' : 
          Serial.println('b');
          changeLEDstatus(&blu_LED);
          break;
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  red_LED.LED_status_Mutex = xSemaphoreCreateMutex();
  if(red_LED.LED_status_Mutex == NULL){return;}
  blu_LED.LED_status_Mutex = xSemaphoreCreateMutex();
  Serial.println("started");
  if(blu_LED.LED_status_Mutex == NULL){return;}
  xTaskCreate(LED_task,  "RED", 128, &red_LED, 1, &task_red_LED);
  xTaskCreate(LED_task, "BLUE", 128, &blu_LED, 1, &task_blu_LED);
  xTaskCreate(serialReadTask, "SERIAL", 128, NULL, 1, &task_serialRead);

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:  
}
