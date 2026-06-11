#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
 
void setup() {
  lcd.begin(16, 2);
  lcd.clear();
}
 
int value = 0;
void loop() {
  lcd.clear();
  value = analogRead(A0);
  lcd.setCursor(0, 1);
  lcd.print(value);
  delay(1000);
}