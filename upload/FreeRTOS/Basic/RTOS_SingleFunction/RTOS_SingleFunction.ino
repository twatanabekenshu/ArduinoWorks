#include <Arduino_FreeRTOS.h>

void task(void* arg){
  char* str = (char*) arg;
  /*
  while(1){
    Serial.println(str);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }*/
  /*
  vTaskDelayUtil(
    TickType_t* pxPreviousWakeTime, : 前回実行時間を記録する変数へのポインタ
    TickType_t  xTimeIncrement      : 時間間隔(単位はtick)を表す変数
    );
    実行する度にpxPreviousWakeTimeの値を変えなければならないが、
    アドレスを指定することで書き換えも自動で行ってくれる
  */
  TickType_t xPreviousWakeTime = xTaskGetTickCount();
  while(1){
    Serial.println(str);
    vTaskDelayUntil(&xPreviousWakeTime ,1000/portTICK_PERIOD_MS);
  }  

}

TaskHandle_t print_TASK_A;
TaskHandle_t print_TASK_B;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  xTaskCreate(task, "TASK A", 128, (void*)"task A", 2, &print_TASK_A);
  xTaskCreate(task, "TASK B", 128, (void*)"task B", 1, &print_TASK_B);

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:

}
