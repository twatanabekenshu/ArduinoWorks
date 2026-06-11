
/*
9ピン,10ピンにつないだLEDが0.5秒間隔で交互に光る
3ピン,4ピンにつないだLEDも同じ間隔で光る. 多分誤差が出る？
*/
unsigned int time_run = 0;
unsigned int time_now = 0;
void setup() {
  // put your setup code here, to run once:
  //(0 << 4)などの論理和は計算上、不要 
  //9pin, 10pin(PB1(timer1A), PB2(timer1B))
  DDRB |= (1 << PB1) | (1 << PB2);
  //0B11100010=226になるはず
  TCCR1A = (1 << COM1A1)|(1 << COM1A0)|(1 << COM1B1)|(0 << COM1B0)|(1 << WGM11)|(0 << WGM10);
  //0B00011101=29になるはず
  TCCR1B = (1 << WGM13)|(1 << WGM12)|(1 << CS12)|(0 << CS11)|(1 << CS10);
  ICR1 = 15624;
  OCR1A = 7812;
  OCR1B = 7813;
  Serial.begin(9600);
  Serial.print("TCCR1A : "); Serial.println(TCCR1A);
  Serial.print("TCCR1B : "); Serial.println(TCCR1B);

  DDRD |= (1 << PD3) | (1 << PD4);
  PORTD|= (1 << PD3) | (0 << PD4);
  time_run = millis();
}


void loop() {
  // put your main code here, to run repeatedly:
  time_now = millis();
  if(time_now - time_run >= 500){
    PORTD^= (1 << PD3) | (1 << PD4);
    Serial.println("Serial.println() is so slow that it takes a few milliseconds to execute.");
    time_run += 500;
  }
}
