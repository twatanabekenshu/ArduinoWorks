#define BUZZER_PIN 7

void beepCorrectAnswerSound();
void beepIncorrectAnswerSound();
void beepIncorrectAnswerSound_2();

void setup(){

}

void loop(){
  
}

//せいかいのおと
void beepCorrectAnswerSound(){
  for(int i = 0; i < 3; i ++){
    tone(BUZZER_PIN, 1864.655);
    delay(200);
    noTone(BUZZER_PIN);
    tone(BUZZER_PIN, 1479.978);
    delay(200);
  }
  noTone(BUZZER_PIN);
}
//不正解の音
void beepInCorrectAnswerSound(){
  for(int i = 0; i < 1; i ++){
    tone(BUZZER_PIN, 277.183);
    delay(200);
    noTone(BUZZER_PIN);
    delay(50);
    tone(BUZZER_PIN, 277.183);
    delay(400);
  }
  noTone(BUZZER_PIN);
}

//不正解の音2
void beepInCorrectAnswerSound_2(){
  for(int i = 0; i < 1; i ++){
    tone(BUZZER_PIN, 698.456);
    delay(200);
    noTone(BUZZER_PIN);

    tone(BUZZER_PIN, 622.254);
    delay(200);
    noTone(BUZZER_PIN);

    tone(BUZZER_PIN, 493.883);
    delay(200);
    noTone(BUZZER_PIN);
  }
}
