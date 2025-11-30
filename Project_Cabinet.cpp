#include <Arduino.h>
#include <softWareSerial.h>

SoftwareSerial mySerial(9,8); // UART : 9(RX), 8(TX)
byte length = 0;

const int PIN_LOCK = 7;
const int PIN_FAN = 6;
const int PIN_LM35 = A0;
const int PIN_MAG = 5;
boolean itsLOCK = true;  // 잠김 : 1(Lock) 0(UNLock)
boolean itsOPEN = false; // 열림 : 1(Open) 0(Close)
float tempture = 0;

// 초음파
const int pinTrig = 4;
const int pinEcho = 3;
int rangeMax = 20; // 20cm 감지
int rangeMin = 0;
long T, L;

int inside = 0; // 안에 있음 여부 0 Empty

volatile byte command = 0;
volatile boolean IhavSel = 0;
volatile boolean complet_JOB = false; // 루틴 확인용
boolean LOCK_OFF = false; // SPI 명령. 

int MEMU = 1; // 작업 선택

void setup() {
  Serial.begin(9600); // Monitoring
  mySerial.begin(9600); // UART Init 

  // SPI SPCR Set
  SPCR |= _BV(SPIE);
  SPCR |= _BV(SPE);
  SPCR &= ~_BV(MSTR);

  // 솔레노이드 & 팬
  pinMode(PIN_LOCK, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  digitalWrite(PIN_LOCK, HIGH);
  digitalWrite(PIN_FAN, HIGH);

  // 초음파
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);

  // 마그네틱
  pinMode(PIN_MAG, INPUT);
}

void loop() {
  if(IhavSel == 1){ // 선택됨
    delay(100);
    // // 1-1. 내부 초음파 계산 
    // digitalWrite(pinTrig, LOW);   delayMicroseconds(2);
    // digitalWrite(pinTrig, HIGH);  delayMicroseconds(10);
    // digitalWrite(pinTrig, LOW);
    // T = pulseIn(pinEcho, HIGH);
    // L = T/58.82;
    // Serial.print("Length : ");
    // Serial.println(L); // Monitering
    // delay(1000);

    // 1-1. 초음파 UART init
    if(mySerial.available()){
      length = mySerial.read(); // L == length
      Serial.print("Length : ");
    }

    // 1-1-1. itsInside? (초음파)
    if (length >= rangeMax || length <= rangeMin) 
      inside = 1; // 있으면 1
    else 
      inside = 0; // 없으면 0
    
    // 1-2. Mgnetic (OPEN 7 : CLOSE 0)
    itsOPEN = digitalRead(PIN_MAG);
    Serial.print("PIN_MAG : ");
    Serial.println(itsOPEN); // Monitoring
    delay(10);

    // 1-3. LM35 Tempture
    tempture = (analogRead(PIN_LM35) * 500.0) / 1024.0;

    // 선택
    if (MEMU == 1){ // 초기 넣는 과정
      // 너 선택된거야
      if(itsLOCK == 1){
        itsLOCK = 0;
        digitalWrite(PIN_LOCK, HIGH);
      } // OPEN 열려

      if(inside == 1 && itsOPEN == 0){ // 안에 넣고 & 문이 닫히면?
        itsLOCK = 1; //잠김
        MEMU = 2; // Sequnce 2로 진행
      }

    } else if (MEMU = 2) { // 넣고 빼는 과정

      if(LOCK_OFF == HIGH){ // 열라고 하면? 열어
        itsLOCK = 0;
      }

      if(itsLOCK == 0 && inside == 0 && itsOPEN == 0) {
        complet_JOB = true;
        MEMU = 1;
      }

      // etc.) FAN control
      if (tempture > 23){ // 23 이상이면
        digitalWrite(PIN_FAN, LOW);
      } else { // 내부에 있을때 온도 높으면 팬 작동
        digitalWrite(PIN_FAN, HIGH);
      }
    }

    // 1. 종료 : 끝났으면?
    if(complet_JOB) { // 일이 끝났으면
      pinMode(MISO, OUTPUT); // MISO ON

      delay (1000); // 통신 대기

      pinMode(MISO, INPUT); // MISO OFF
    }
  }
}

ISR(SPI_STC_vect){
  byte c = SPDR; // BUFFER

  if (c == 's'){
    IhavSel = 1;
    return;
  } else if (c == 'p') {
    LOCK_OFF = HIGH; // 인증 완료 열어야됨
    return;
  }

  switch(command){
    case 0 : 
      command = c;
      SPDR = 0;
      break;

    case 'c': 
      SPDR = complet_JOB; // 저 끝남요
      break;
  }
  
}