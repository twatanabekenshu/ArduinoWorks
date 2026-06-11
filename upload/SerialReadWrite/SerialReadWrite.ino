#define   setBit(x, y)  ((x) |=  (1 << y))
#define clearBit(x, y)  ((x) &= ~(1 << y))

char readChar;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setBit(DDRB, PB5);
  clearBit(PORTB,PB5);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0){
    readChar = Serial.read();
    if(readChar == '\n' || readChar == '\r'){
      return ;
    }

    if(readChar == '0'){
      clearBit(PORTB,PB5);
      Serial.println("LED OFF");
      return ;
    }
    
    if(readChar == '1'){
      setBit(PORTB,PB5);
      Serial.println("LED ON");
      return ;
    }
    
    Serial.println("Invalid input ! Input character must be 0 or 1 !");

  }
}
