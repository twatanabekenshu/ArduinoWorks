#include <Arduino_FreeRTOS.h>

#define LED_PIN 13

TaskHandle_t taskBlink_handler;
TaskHandle_t taskSerial_handler;

void taskBlink(void* args);
void taskSerial(void* args);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);

  Serial.println("s : suspend, r : resume");
  /*
  xTaskCreate(
    void(*func),         : タスクとして行う関数(戻り値なし、引数はvoid*), 原則、無限ループとする
    char[] taskName,     : タスクの名前(デバッグ時に有用)
    unsigned? stackSize, : このタスクに割り当てるスタックサイズ(あまり大きくすると壊れる, グローバル変数込みで約2000なので、)
    void* args,          : タスク関数の引数
    unsigned? priority,  : このタスクの優先順位(数値が大きいほど高い)
    TaskHandle_t* hander : このタスクハンドラーを格納するための変数のアドレス
  );
  */
  xTaskCreate( taskBlink,  "Blink", 128, NULL, 2, &taskBlink_handler);
  xTaskCreate(taskSerial, "Serial", 128, NULL, 1, &taskSerial_handler);
  Serial.print("portTICK_PERIOD_MS");Serial.println(portTICK_PERIOD_MS);
  vTaskStartScheduler(); 
}

void loop() {
  // FreeRTOSが行ってくれるので、処理の記述不要

}

/*
vTaskSuspend(TaskHandler* handler)
  handlerに格納されたタスクを中断する
vTaskResume(TaskHandler* handler)
  handlerに格納されたタスクを再開する

portTICK_PERIOD_MS
  FreeRTOSにおけるタイマー割り込みの単位を「tick(ティック)」という。
  このtickが現実の何ミリ秒に当たるかを表すマクロ(今回は1ミリ秒)
vTaskDelay(ticks)
  ticks : FreeRTOSで何ティック分実行スケジュールから外すかを指定する。
  delay()と違い、ノンブロッキングに待つことができる。
  マルチタスク環境でdelay()を行うとほかの処理も止まるので、好ましくない。
*/

void taskBlink(void* args){
  pinMode(LED_PIN, HIGH);
  while(1){
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void taskSerial(void* args){
  while(1){
    if(Serial.available() > 0){
      switch(Serial.read()){
        case 's' : 
          Serial.println("Task was suspended");
          vTaskSuspend(taskBlink_handler);
          break;
        case 'r' : 
          Serial.println("Task was resumed");
          vTaskResume(taskBlink_handler);
          break;
      }
    }
    vTaskDelay(1);
  }
}
