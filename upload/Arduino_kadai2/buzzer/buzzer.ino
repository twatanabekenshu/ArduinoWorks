#include <Arduino_FreeRTOS.h>
//#include <semphr.h>

#define MAX_LENGTH 100

typedef struct {
  unsigned int soundData[17][2];
  unsigned int length;
  unsigned int tempo;
}Music;

#define L 330
#define M 440
#define H 550

Music panorama;

int length = 2;
int tempo = 600;

//unsigned int panoramaSoundData[18][2] = {{L,1},{H,1},{M,1},{L,1},{H,1},{M,1},{L,1},{H,1},{L,1},{M,2},{0,1},{M,1},{L,1},{H,1},{M,1},{L,1},{H,1},{M,2}};
#define BTN_PIN 2
TaskHandle_t music_task;

unsigned int soundData[2][2] = {{1568,1},{1319,1}};

void button_check_func(void* args){
  //車道が青の場合に限り、ボタンの状態チェックを行う。押された瞬間(HIGH->LOW->LOW)を検出する
  bool    last_btn_state = HIGH;
  bool current_btn_state = HIGH;
  
  while(1){
    current_btn_state = digitalRead(BTN_PIN);
    if(current_btn_state == LOW && last_btn_state == HIGH){
      vTaskDelay(1);
      current_btn_state = digitalRead(BTN_PIN);
      if(current_btn_state == LOW){
          last_btn_state = LOW;
          xTaskNotifyGive(music_task);
        }
      }
    last_btn_state = current_btn_state;
    vTaskDelay(1);
  }
}

void music(void* args){
  while(1){
    for(int i = 0; i < 2; i ++){
      if(soundData[i][0] == 0){
        noTone(6);
      }else{
        tone(6, soundData[i][0]);
      }
      //途中中止をイベントフラグで実装
      if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(soundData[i][1]*tempo))!=0){break;}
    }
    noTone(6);
    vTaskDelay(1);
  }
}

void setup() {
  // put your setup code here, to run once:
  xTaskCreate(music, "MUSIC", 128, 1, NULL, &music_task);
  //xTaskCreate(button_check_func, "BTN", 128, 1, NULL, NULL);
  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
