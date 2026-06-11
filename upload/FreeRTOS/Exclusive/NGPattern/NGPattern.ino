#include <Arduino_FreeRTOS.h>
#include <MsTimer2.h>

/*
グローバル変数を処理中に書き換えるため、不整合が起きる例
taskA・taskBの2つのタスクを実行している。
どちらのタスクもfunction(int x, char c)という関数を呼び出した後にその関数の戻り値resを「タスク名:戻り値の値はres」という形式で表示している。
このfunctionでは次のような操作が行われている。
1. 「c -> x」という形式で表示する。
2. xで受け取った整数をtempに書き込む。
3. delay(3000)ののちにtempの値を返す。
このdelay(3000)の間にtempが書き換えられてしまうため、resが最初に渡した数でなくなる。
*/
int temp;
float count = 0;

TaskHandle_t TaskA_Handler;
TaskHandle_t TaskB_Handler;

void setup() {
  Serial.begin(9600);
  Serial.println("enable control");

  MsTimer2::set(500, funcCount);
  MsTimer2::start();
  
  xTaskCreate(TaskA, "A", 128, NULL, 1, &TaskA_Handler );
  xTaskCreate(TaskB, "B", 128, NULL, 1, &TaskB_Handler );

  vTaskStartScheduler();
}
    
void loop(){}

void funcCount(){
  count = count + 0.5;
  Serial.println(count);
}

void TaskA(void *pvParameters){
  while(1){
    int res = function(5, 'A');
    Serial.print("A：返り値は ");
    Serial.println(res);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void TaskB(void *pvParameters){
  while(1){
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    int res = function(8, 'B');
    Serial.print("B：返り値は ");
    Serial.println(res);
  }
}

int function(int x, char c){
  Serial.print(c);
  Serial.print(" → ");
  Serial.println(x);
  temp = x;
  delay(3000);
  return temp;
}
