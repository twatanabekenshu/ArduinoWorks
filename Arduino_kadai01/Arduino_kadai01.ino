#define     LEDPIN PB0 //Arduino  8pin
#define   LATCHPIN PB4 //Arduino 12pin
#define   CLOCKPIN PB5 //Arduino 13pin
#define  BUTTONPIN PD2 //Arduino  2pin
#define BUZZER_PIN PD3 //Arduino  3pin
#define  SERVO_PIN PB1 //Arduino  9pin

#define    RGB_RED PD5 //Arduino  5pin
#define    RGB_GRN PD6 //Arduino  6pin
#define    RGB_BLU PD7 //Arduino  7pin

#define clr_bit(target, no) ((target) &= ~(0B00000001 << (no)))
#define set_bit(target, no) ((target) |=  (0B00000001 << (no)))

//LEDの流れる間隔の最大値と最小値を設定
#define INTERVAL_MAX 230
#define INTERVAL_MIN  50

//タイムアウトになるまでのカウント(LED8個なので10周)
#define TIME_OUT_CNT  80
//LEDが動くごとに1つずつカウントが減る
byte time_cnt = TIME_OUT_CNT;

//構造体:周期的なタスクを表す
/*
unsigned long pre_runTime : 前回実行した時刻(ms)
unsigned long    interval : 時間間隔(ms)
          void task(void) : 実行するタスクへの関数ポインタ(戻り値・引数ともになし)
          bool is_running : このタスクを実行しなければならないか
*/
typedef struct {
  unsigned long pre_runTime;
  unsigned long interval;
  void (*task)(void);
  volatile bool is_running;
}periodicTask;
/*
unsigned long time_now : 現在の時刻を記録
*/
unsigned long time_now;
int targetLED_num;
byte lightingLED = 0B00000000;

void handlePeriodicTask(periodicTask*);
void btnCheck();
void timeoutCheck();
void LED_success();
void LED_miss();
void controlLED();
void clearLED();
void lightingLEDUpdate();
void setTarget();
void beepCorrectAnswerSound();
void beepIncorrectAnswerSound();
void myServo(int);
int interval_to_degree(unsigned long);
void myTone(double);
void myNoTone();
void RGB_init();
void RGB_off();
void RGB_red();
void RGB_blue();
void RGB_rg(byte, byte);

periodicTask lightingLEDUpdate_task = {0, INTERVAL_MAX, &lightingLEDUpdate, true};
periodicTask btnCheck_task = {0, 1, &btnCheck, true};
periodicTask timeoutCheck_task = {0, 1, &timeoutCheck, true};

void setup() {
  //シフトレジスタにつなぐピンを出力ピンに設定する
  set_bit(DDRB, LEDPIN);
  set_bit(DDRB, LATCHPIN);
  set_bit(DDRB, CLOCKPIN);
  //2ピンを入力にし、内部プルアップありとする
  clr_bit(DDRD, BUTTONPIN);
  set_bit(PORTD,BUTTONPIN);
  //タイマー1でサーボモーターを使うための設定
  TCCR1A = 0B10000010;
  //OC1A(=9pin)でPWM(非反転、比較一致でLOW)
  TCCR1B = 0B00011010;
  //WGM:高速PWM, TOPはICR1/分周比:8
  ICR1   = 40000 - 1;
  //9pinを出力にする
  set_bit(DDRB, SERVO_PIN);
  //サーボモーターを初期位置に
  myServo(interval_to_degree(lightingLEDUpdate_task.interval));
  setTarget();

  //BUZZER_PINを出力に設定し、LOWにしておく
  DDRD |= (1 << BUZZER_PIN);
  PORTD&= ~(1 << BUZZER_PIN);
  //3pin(BUZZER_PIN)をPWMモードに設定
  TCCR2A= 0B00100011;
  //そのうえでWGM2を111に設定し、OCR2AでTCNT2がリセットされるPWMにする.
  //ただし、最初はこのカウンタを動かさない
  TCCR2B= 0B00001000;
  //RGB LEDのための初期設定
  RGB_init();
}

void loop() {
  time_now = millis();
  handlePeriodicTask(&btnCheck_task);
  handlePeriodicTask(&timeoutCheck_task);
  handlePeriodicTask(&lightingLEDUpdate_task);
}
//周期的に行うタスクを処理する関数
void handlePeriodicTask(periodicTask* pt){
  //前回の処理時刻と現在時刻を比べ、時間間隔を超えてるときに時刻を
  if(time_now - pt->pre_runTime >= pt->interval){
    pt->pre_runTime = time_now;
    //タスクが継続中なら実行
    if(pt->is_running){
      pt->task();
    }
  }
}

void btnCheck(){
  if(!(PIND & (1 << BUTTONPIN))){
    lightingLEDUpdate_task.is_running = !lightingLEDUpdate_task.is_running;
    
    //ボタン操作で止めた場合
    if(!lightingLEDUpdate_task.is_running){
      if(lightingLED & (1 <<targetLED_num)){
        //成功
        LED_success();
      }else{
        //失敗
        RGB_red();
        LED_miss();
      }
    }
    //ボタンが離されるまで進まない
    do{
      delay(100);
    }while(!(PIND & (1 << BUTTONPIN)));
    lightingLEDUpdate_task.is_running = true;
    delay(100);
  }
}

void timeoutCheck(){
  if(time_cnt == 0){
    LED_miss();
    setTarget();
    time_cnt = TIME_OUT_CNT;
  }
}

void LED_success(){
  time_cnt = TIME_OUT_CNT;
  RGB_blue();
  beepCorrectAnswerSound();
  //点灯のタイミングを速くする
  lightingLEDUpdate_task.interval -= 10;
  //下限をINTERVAL_MINとする
  if(lightingLEDUpdate_task.interval < INTERVAL_MIN){
    lightingLEDUpdate_task.interval = INTERVAL_MIN;
  }
  RGB_off();
  myServo(interval_to_degree(lightingLEDUpdate_task.interval));
  setTarget();
}

void LED_miss(){
  RGB_red();
  beepIncorrectAnswerSound();
  //点灯のタイミングを遅くする
  lightingLEDUpdate_task.interval += 10;
  //上限はINTERVAL_MAXとする
  if(lightingLEDUpdate_task.interval > INTERVAL_MAX){
    lightingLEDUpdate_task.interval = INTERVAL_MAX;
  }
  RGB_off();
  myServo(interval_to_degree(lightingLEDUpdate_task.interval));
  
}

//MSB FIRSTで8,12,13につながった74HC595からbyte lightingLEDに従ってLEDを光らせる
void controlLED(){
  clr_bit(PORTB, LATCHPIN);
  byte mask = 0B10000000;
  for(int i = 0; i < 8; i ++){
    if(lightingLED & mask){
      set_bit(PORTB, PB0);
    }else{
      clr_bit(PORTB, PB0);
    }
    mask = mask >> 1;
    set_bit(PORTB, CLOCKPIN);
    clr_bit(PORTB, CLOCKPIN);
  }
  set_bit(PORTB, LATCHPIN);
}

//全LEDを消灯する
void clearLED(){
  clr_bit(PORTB, LATCHPIN);
  for(int i = 0; i < 8; i ++){
    clr_bit(PORTB, PB0);
    set_bit(PORTB, CLOCKPIN);
    clr_bit(PORTB, CLOCKPIN);
  }
  set_bit(PORTB, LATCHPIN);
}

//点灯させるLEDのデータを更新し、そのLEDを点灯する
void lightingLEDUpdate(){
  if(time_cnt > 0){time_cnt --;}
  RGB_rg(3*(TIME_OUT_CNT-time_cnt), 3*time_cnt);
  lightingLED = lightingLED << 1;
  if(lightingLED == 0){lightingLED = 1;}
  controlLED();
}

//正解のピンを指定:何もつないでいないA0からは不定な値が来るので、その入力を乱数として扱う
void setTarget(){  
  targetLED_num = analogRead(A0) % 8;
  lightingLED = (1 << targetLED_num);
  for(int i = 0; i < 3; i ++){
    controlLED();
    delay(1000);
    clearLED();
    delay(1000);
  }
  //どのLEDから始まるかは時間に合わせて指定
  lightingLED = (1<<millis()%8);
}

//正解の時に音を鳴らす(ピンポンと3回鳴る)
void beepCorrectAnswerSound(){
  for(int i = 0; i < 3; i ++){
    myTone(1864.655);
    delay(200);
    myNoTone();
    myTone(1479.978);
    delay(200);
  }
  myNoTone();
}

//不正解の時に音を鳴らす(ブッブーと1回鳴る)
void beepIncorrectAnswerSound(){
  myTone(220.0);
  delay(200);
  myNoTone();
  delay(50);
  myTone(220.0);
  delay(400);
  myNoTone();
}

//サーボモーターをdegで指定した角度に動かす
void myServo(int deg){
  if(-90 <= deg && deg <= 90){
    OCR1A = 2900 + 190.0 * deg /9 - 1;
  }
}

//LEDの流れる間隔とサーボモーターの角度を対応させる
int interval_to_degree(unsigned long interval){
  return ((180.0*(interval - INTERVAL_MIN)/(INTERVAL_MAX - INTERVAL_MIN)) - 90);
}

void myNoTone(){
  //
  TCCR2B &= 0B11111000;
  PORTD  &= ~(1 << BUZZER_PIN);
  OCR2B   = 0;
}

void myTone(double freq){
  //可聴域外なので何もしない
  if(freq > 20000){
    myNoTone();
    return;
  }
  if(freq > 8000){
    //分周比8
    TCCR2B = 0B00001010;
    OCR2A = 2000000/freq-1;
    OCR2B = OCR2A/2;
    return;
  }

  if(freq > 2000){
    //分周比32
    TCCR2B = 0B00001011;
    OCR2A = 500000/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  //分周比64は対応範囲が狭いため使用しない

  if(freq > 500){
    //分周比128
    TCCR2B = 0B00001101;
    OCR2A = 125000/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  if(freq > 250){
    //分周比256
    TCCR2B = 0B00001110;
    OCR2A = 62500/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  if(freq > 62.5){
    //分周比1024
    TCCR2B = 0B00001111;
    OCR2A = 15625/freq-1;
    OCR2B = OCR2A/2;
    return;
  }
  //範囲外//音を出さない
  myNoTone();
  return;
}

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
