#include <avr/wdt.h>

#define btninputPin 2

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  /*
  MCUSR:micro controler status register
  前回のリセット原因を格納するレジスタ
  
  WDRF:watch dog reset flag
  MCUSR内にあるwatch dog timerでリセットされたことを表すフラグ
  */
  Serial.println("launched");
  if(bitRead(MCUSR, WDRF)){
    Serial.println("Reset by WDT");
    Serial.println();
  }

  wdt_enable(WDTO_4S);

  DDRD |= 0B00000100;
  PORTD|= 0B00000100;
}

void loop() {
  // put your main code here, to run repeatedly:
  int cnt = 0;
  
  while(PIND & 0B00000100){
    cnt ++;
    Serial.print(cnt);
    Serial.println(" seconds passed ");
    delay(1000);
  }

  Serial.print(millis()/1000);
  Serial.println(" seconds passed from started");
  wdt_reset();
  delay(100);
}
