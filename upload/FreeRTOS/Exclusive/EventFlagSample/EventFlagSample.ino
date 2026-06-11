/*
【バイナリセマフォ】
xSemaphoreCreateBinary()
通知・フラグ用途

【カウントセマフォ】
xSemaphoreCreateCounting()
リソース数管理

【ミューテックス】
xSemaphoreCreateMutex()
排他制御、優先度継承

【再帰的ミューテックス】
xSemaphoreCreateRecursiveMutex()
ネストロック対応
*/

#include <Arduino_FreeRTOS.h>
#include <event_groups.h>  // イベントグループ用のヘッダをインクルード

const int LED_TaskA = 13;
const int LED_TaskB = 11;
const int LED_TaskC = 8;
const int LED_TaskD = 3;

const int  TASK_A_FLAG = (1 << 0);   // タスクA用のイベントフラグ     (0b00000001)
const int  TASK_B_FLAG = (1 << 1);   // タスクB用のイベントフラグ     (0b00000010)
const int  TASK_C_FLAG = (1 << 2);   // タスクC用のイベントフラグ     (0b00000100)
const int TASK_D1_FLAG = (1 << 3);   // タスクD1用のイベントフラグ(1) (0b00001000)
const int TASK_D2_FLAG = (1 << 4);   // タスクD2用のイベントフラグ(2) (0b00010000)
const int  TASK_D_FLAG = TASK_D1_FLAG | TASK_D2_FLAG;
/*
TASK_D_FLAGの設定
元のプログラムでは「TASK_D_FLAG = 0x24;」と設定されていたが、
TASK_D_FLAGはTASK_D1_FLAGとTASK_D2_FLAGが両方立っている状態を表すので、
0x24(0b00100100, 36)ではなく、TASK_D1_FLAG | TASK_D2_FLAG( = 0x18(0b00011000, 24))のほうが適切?
*/
/*
タスクA:
このタスクはeventGroupのTASK_A_FLAGが立っているときに行われる。
シリアルモニタに「[TaskA Start]」と表示し、タスクA用LEDを点灯して5秒待つ。
その後、シリアルモニタに「[TaskA End]」と表示する。
最後にeventGroupのTASK_B_FLAGとTASK_C_FLAGを立てる。

タスクB:
このタスクはeventGroupのTASK_B_FLAGが立っているときに行われる。
シリアルモニタに「[TaskB Start]」と表示し、タスクB用LEDを点灯して10秒待つ。
その後、シリアルモニタに「[TaskB End]」と表示する。
最後にeventGroupのTASK_D1_FLAGを立てる。

タスクC:
このタスクはeventGroupのTASK_C_FLAGが立っているときに行われる。
シリアルモニタに「[TaskC Start]」と表示し、タスクC用LEDを点灯して5秒待つ。
その後、シリアルモニタに「[TaskC End]」と表示する。
最後にeventGroupのTASK_D2_FLAGを立てる。

タスクD:
このタスクはeventGroupのTASK_D1_FLAGとTASK_D2_FLAGが両方立っているときに行われる。
シリアルモニタに「[TaskD Start]」と表示し、タスクD用LEDを点灯して5秒待つ。
その後、シリアルモニタに「[TaskD End]」と表示する。
最後にeventGroupのTASK_A_FLAGを立てる。

このプログラムではsetup()内でeventGroupのTASK_A_FLAGを立ててから開始する。
そのため、TaskA用LEDを赤、TaskB用LEDを青、TaskC用LEDを緑、TaskD用LEDを黄とすると
赤点灯→5秒後に赤消灯、青と緑が同時に点灯開始->5秒後に緑消灯->さらに5秒後に青消灯、黄点灯→5秒後に黄色消灯→赤点灯に戻る
というサイクルでLEDが点灯・消灯してゆく。
*/

EventGroupHandle_t eventGroup;  // イベントグループのハンドルを定義

TaskHandle_t TaskA_Handler;
TaskHandle_t TaskB_Handler;
TaskHandle_t TaskC_Handler;
TaskHandle_t TaskD_Handler;

void setup() {
  pinMode( LED_TaskA, OUTPUT );
  pinMode( LED_TaskB, OUTPUT );
  pinMode( LED_TaskC, OUTPUT );
  pinMode( LED_TaskD, OUTPUT );

  Serial.begin(9600);
  Serial.println("***START***");

  eventGroup = xEventGroupCreate();   // イベントグループの作成
  
  xTaskCreate(TaskA, "A", 128, NULL, 1, &TaskA_Handler );
  xTaskCreate(TaskB, "B", 128, NULL, 1, &TaskB_Handler );
  xTaskCreate(TaskC, "C", 128, NULL, 1, &TaskC_Handler );//<-元はTaskB_HandlerだったがTaskC_Handlerのほうが適切?
  xTaskCreate(TaskD, "D", 128, NULL, 1, &TaskD_Handler );//<-元はTaskB_HandlerだったがTaskD_Handlerのほうが適切?

  xEventGroupSetBits(eventGroup, TASK_A_FLAG);  // タスクAを最初に開始させる

  vTaskStartScheduler();
}
    
void loop(){}

void TaskA(void *pvParameters){
  while(1){
    // タスクAがフラグを取得するまで待機
    xEventGroupWaitBits(
      eventGroup,       //イベントグループのハンドル（識別子）
      TASK_A_FLAG,      //タスクAが待機しているビットフラグ。タスクAはこのフラグがセットされるのを待つ。
      pdTRUE,           //タスクAがこのビットをキャッチした後に、そのビットを0にする。->後処理も考えてある
      pdFALSE,          //単一のビット（TASK_A_FLAG）のみがセットされるのを待つ。すべてのビットを待つわけではない。
      portMAX_DELAY     //ビットがセットされるまでの待機時間。portMAX_DELAYだと無限に待機しタスクはずっとブロック状態になる。
    );

    Serial.println("[TaskA Start]");
    digitalWrite( LED_TaskA, HIGH );
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    digitalWrite( LED_TaskA, LOW );
    Serial.println("[TaskA End]");

    // タスクBのフラグを設定してタスクBを解放
    xEventGroupSetBits(
      eventGroup,       //操作対象のイベントグループのハンドル（識別子）
      TASK_B_FLAG       //イベントグループ内で1をセットしたいビットを指定する値
    );

    // タスクCのフラグを設定してタスクCを解放
    xEventGroupSetBits(eventGroup, TASK_C_FLAG);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskB(void *pvParameters){
  while(1){
    // タスクBがフラグを取得するまで待機
    xEventGroupWaitBits(eventGroup, TASK_B_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);

    Serial.println("[TaskB Start]");
    digitalWrite( LED_TaskB, HIGH );
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    digitalWrite( LED_TaskB, LOW );
    Serial.println("[TaskB End]");

    // タスクDのフラグを設定(D1)
    xEventGroupSetBits(eventGroup, TASK_D1_FLAG);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskC(void *pvParameters){
  while(1){
    // タスクCがフラグを取得するまで待機
    xEventGroupWaitBits(eventGroup, TASK_C_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);

    Serial.println("[TaskC Start]");
    digitalWrite( LED_TaskC, HIGH );
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    digitalWrite( LED_TaskC, LOW );
    Serial.println("[TaskC End]");

    // タスクDのフラグを設定(D2)
    xEventGroupSetBits(eventGroup, TASK_D2_FLAG);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskD(void *pvParameters){
  while(1){
    // タスクDがフラグ(D1,D2)を取得するまで待機
    xEventGroupWaitBits(eventGroup, TASK_D_FLAG, pdTRUE, pdTRUE, portMAX_DELAY);
    // xEventGroupWaitBits(eventGroup, 24(0B00011000), pdTRUE, pdTRUE, portMAX_DELAY);
    // TASK_D1_FLAG | TASK_D2_FLAG　= 0b00011000なので0x18=24?

    Serial.println("[TaskD Start]");
    digitalWrite( LED_TaskD, HIGH );
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    digitalWrite( LED_TaskD, LOW );
    Serial.println("[TaskD End]");

    // タスクAのフラグを設定
    xEventGroupSetBits(eventGroup, TASK_A_FLAG);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
