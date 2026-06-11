void setup() {
  // put your setup code here, to run once:
  DDRD &= ~(1 << PD4);//~(1 << PD4)=11101111なので、第4ビットだけ0にして他は触らない
  PORTD|= (1 << PD4);
  DDRD |= 1 << PD3;//00001000なので、第3ビットだけ1にして他は触らない
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  Dグループのピンを読み込んだ結果はPINDに書かれる。
  フォーマットはnピンが読み込みに設定されていてHIGH(5V)なら第nビットを1にし、そのほかの場合には0にする。
  このスケッチではPINDの第4ビットを取り出したいので、1<<PD4=00010000と論理積を取る
  */
  if(!(PIND & (1 << PD4))){
    PORTD |= 1 << PD3;
  }else{
    PORTD &= ~(1 << PD3);
  }
}
