#define CDSPIN A0

#define LED_WAIT_MILLIS_DARK     10
#define LED_WAIT_MILLIS_BRIGHT   50
#define CDS_INTERVAL_MILLIS     100
#define SERIAL_INTERVAL_MILLIS 1000

#define CDS_BRIGHT_THRESHOLD    512
#define CDS_DARK_THRESHOLD      448

typedef struct{
  uint32_t pre_run_time;
  uint32_t interval;
  void (*function)(void*);
  void* args;
}periodicTask_t;

void readCdS_value(void*);
void sendCdS_value(void*);
void pwmLED(void*);
void handlePeriodicTask(periodicTask_t*);

/*
Board : Arduino UNO
Connection :
   3 pin - 330 ohm resister - LED - GND
  5V - 10K ohm resistor-- CdS - GND
                        |- A0 pin
*/
int pwmDuty = 0;
int dir = 1;
unsigned int CdS_value = 0;
unsigned int CdS_threshold = CDS_BRIGHT_THRESHOLD;

unsigned long time_now;

periodicTask_t led_pwm = {0, LED_WAIT_MILLIS_BRIGHT, &pwmLED, NULL};
periodicTask_t cds_read = {0, CDS_INTERVAL_MILLIS, &readCdS_value, &led_pwm.interval};
periodicTask_t cds_send = {0, SERIAL_INTERVAL_MILLIS, &sendCdS_value, NULL};

void setup() {
  // put your setup code here, to run once:
  DDRD |= 0b00001000;
  TCCR2A = 0B00100011;
  TCCR2B = 0B00000001;
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //一本道なことには変わりないので、1つの重い処理でブロックされる可能性がある
  time_now = millis();
  handlePeriodicTask(&cds_read);
  handlePeriodicTask(&cds_send);
  handlePeriodicTask(&led_pwm);
  
}

void handlePeriodicTask(periodicTask_t* task){
  if((time_now - (task->pre_run_time)) >= task->interval){
    task->function(task->args);
    task->pre_run_time += task->interval;
  }
}

void readCdS_value(void* arg){
  CdS_value = analogRead(CDSPIN);
  if(arg == NULL){return;}
  if(CdS_value > CdS_threshold){
    //暗いとき
    CdS_threshold = CDS_DARK_THRESHOLD;
    *((int*)arg) = LED_WAIT_MILLIS_DARK;
  }else{
    //明るいとき
    CdS_threshold = CDS_BRIGHT_THRESHOLD;
    *((int*)arg) = LED_WAIT_MILLIS_BRIGHT;
  }
}

void sendCdS_value(void* arg){
  Serial.print("Value read from CdS on A0 pin : ");
  Serial.println(CdS_value);
}

void pwmLED(void*  arg){
  OCR2B = pwmDuty;//<-analogWrite(LEDPIN, pwmDuty);
  pwmDuty += dir;

  if(pwmDuty <= 0 || pwmDuty >= 255){
    dir = -dir;
  }
}
