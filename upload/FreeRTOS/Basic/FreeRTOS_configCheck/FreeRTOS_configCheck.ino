#include <Arduino_FreeRTOS.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(configTICK_RATE_HZ);
  Serial.println(portUSE_WDTO);
}

void loop() {
  // put your main code here, to run repeatedly:

}
