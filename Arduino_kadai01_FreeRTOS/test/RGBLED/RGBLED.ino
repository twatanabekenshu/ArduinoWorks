/*
5,6,7でRGB LEDを制御する
・5 : 赤・6 : 緑・7 : 青
本番用実機のピン制約から7はPWM非対応
*/

void RGB_init(){
  DDRD |= 0b11100000;
  TCCR0A |= 0b00000011;
  TCCR0A &= 0b00001111;
  TCCR0B &= 0b11110111;
  OCR0A = 0;
  OCR0B = 0;
  PORTD &= 0b00011111;
}

void RGB_off(){
  TCCR0A &= 0b00001111;
  PORTD  &= 0b00011111;
}

void RGB_red(){
  TCCR0A &= 0b00001111;
  PORTD |= 0b00100000;
  PORTD &= 0b00111111;
}

void RGB_yellow(){
  TCCR0A |= 0b10100000;
  OCR0A = 100;
  OCR0B = 235;
}
void RGB_green(){
  TCCR0A &= 0b00001111;
  PORTD |= 0b01000000;
  PORTD &= 0b01011111;
}

void RGB_blue(){
  TCCR0A &= 0b00001111;
  PORTD |= 0b10000000;
  PORTD &= 0b10011111;
}

void RGB_rg(byte redDuty, byte greenDuty){
  TCCR0A |= 0b10100000;
  OCR0A = greenDuty;
  OCR0B = redDuty;
}

void setup() {
  // put your setup code here, to run once:
  RGB_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  for(byte i = 0; i < 240; i ++){
    RGB_rg(i, 240 - i);
    delay(50);
  }
}
