#include <Arduino_FreeRTOS.h>
#include <semphr.h>

TaskHandle_t blink_task;
TaskHandle_t serialCheck_task;
TaskHandle_t printCount_task;

SemaphoreHandle_t serial_write_mutex;

typedef enum{
  PRIORITY_IDLE = 0,
  PRIORITY_LOW,
  PRIORITY_NORMAL,
  PRIORITY_HIGH,
  PRIORITY_CRITICAL
}Task_PRIORITY;

//1秒ごとにLEDを点滅させる
void blink(void* args){
  DDRB |= 0B00010000;//PORTBの４番ピン(Arduino 12ピン)を出力ピンにする
  //millis()で現在時刻を記録するのと同じノリでやっている
  TickType_t xPreviousWakeTime = xTaskGetTickCount();

  while(1){
    //PORTBの4番ピン(Arduino 12ピン)を反転する
    PORTB ^= 0B00010000;
    //1秒ごとに行うための関数
    //第1引数で前回実行した時刻を記録している
    vTaskDelayUntil(&xPreviousWakeTime, 1000/portTICK_PERIOD_MS);
  }
}

void printCount(void* args){
  unsigned long count = 0;
  //TickType_t xPreviousWakeTime = xTaskGetTickCount();
  while(1){
    count ++;
    if(xSemaphoreTake(serial_write_mutex, pdMS_TO_TICKS(10))==pdTRUE){
      Serial.println(count);
    }
    xSemaphoreGive(serial_write_mutex);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    //止まっている間に溜まってしまう->例:10で止め、5秒後に押すと一気に11~15が出力される
    //vTaskDelayUntil(&xPreviousWakeTime, 1000/portTICK_PERIOD_MS);
  }
}

void serialCheck(void* args){
  
  Serial.println("r : resume, s : suspend");
  while(1){
    if(Serial.available()){
      
        switch(Serial.read()){
          case 'r' : 
          //ここはセマフォにしたほうが良い
            if(xSemaphoreTake(serial_write_mutex, pdMS_TO_TICKS(10))==pdTRUE){
              Serial.println("r received");
            }
            vTaskResume(printCount_task);
            xSemaphoreGive(serial_write_mutex);
            break;
        case 's' : 
        //ここはセマフォにしたほうが良い
          if(xSemaphoreTake(serial_write_mutex, pdMS_TO_TICKS(10))==pdTRUE){
            Serial.println("s received");
          }
          vTaskSuspend(printCount_task);
          xSemaphoreGive(serial_write_mutex);
          break;
      }
    }
    vTaskDelay(1);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  serial_write_mutex = xSemaphoreCreateMutex();
  /*
  優先順位の指定：要件で決める。
  安全上必要なもの,キー入力は上げるなど
  enumで決めておくと自動で値を割り振るのでやりやすく、ぶつからない、ミスりにくい
  */
  xTaskCreate(      blink,       "Blink", 128, NULL, PRIORITY_CRITICAL, &blink_task);       //今回はこれを最優先にするという要件があるので最優先
  xTaskCreate(serialCheck, "SerialCheck", 128, NULL,     PRIORITY_HIGH, &serialCheck_task); //シリアル入力受付はカウント表示より優先度高くしておく
  xTaskCreate( printCount,  "PrintCount", 128, NULL,   PRIORITY_NORMAL, &printCount_task);  //カウント表示は優先度最低

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
