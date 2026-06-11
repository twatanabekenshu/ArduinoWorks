#include <Arduino_FreeRTOS.h>
#include <semphr.h>

/*
優先度逆転が起きることを確かめるためのプログラム
Mutexを使うと優先度継承があるため、BinarySemaphoreにして優先度の逆転をわざと起こす
*/
//優先順位をenumで指定
typedef enum{
  LOW_PRIORITY = 1,
  NORMAL_PRIORITY,
  HIGH_PRIORITY
}Priority;

SemaphoreHandle_t resorceSemaphore;

void task_low_priority(void*){
  //優先度は低いがとても重い処理で、task_high_priorityと同じリソースを使う
  while(1){
    //セマフォ取得
    if(xSemaphoreTake(resorceSemaphore, portMAX_DELAY)==pdTRUE){
      Serial.println("処理task_low_priority()開始");
      vTaskDelay(1500/portTICK_PERIOD_MS);
      Serial.println("処理task_low_priority()終了");
      //セマフォ返却
      xSemaphoreGive(resorceSemaphore);
    }
  }
}
void task_normal_priority(void*){
  //優先度が中程度な処理、特定のリソースは使わない
  while(1){
    Serial.println("処理task_normal_priority()開始");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    Serial.println("処理task_normal_priority()終了");
    vTaskDelay(4000/portTICK_PERIOD_MS);
  }
}
void task_high_priority(void*){
  //優先度が高い処理、task_low_priorityと同じリソースを使う
  Serial.println("処理highの初期化");
  while(1){
    //セマフォ取得
    if(xSemaphoreTake(resorceSemaphore, portMAX_DELAY)==pdTRUE){
      Serial.println("処理task_high_priority()開始");
      vTaskDelay(500/portTICK_PERIOD_MS);
      Serial.println("処理task_high_priority()終了");
      xSemaphoreGive(resorceSemaphore);
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Started ! ");

  resorceSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(resorceSemaphore);
  xTaskCreate(   task_low_priority,    "low_priority", 128, NULL,    LOW_PRIORITY, NULL);
  xTaskCreate(task_normal_priority, "normal_priority", 128, NULL, NORMAL_PRIORITY, NULL);
  xTaskCreate(  task_high_priority,   "high_priority", 128, NULL,   HIGH_PRIORITY, NULL);

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:

}
