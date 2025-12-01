#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(8,9); // UART : 8(RX), 9(TX)

const int PIN_LOCK = 7;
const int PIN_FAN = 6;
const int PIN_LM35 = A4;
const int PIN_MAG = 5;

boolean itsLOCK = true;  // 잠김 : 1(Lock) 0(UNLock)
boolean itsOPEN = false; // 열림 : 1(Open) 0(Close)

// 초음파
const int PIN_Trig = 4;
const int PIN_Echo = 3;
int rangeMax = 10;
int rangeMin = 0;
long T, L;

int inside = 0; // 안에 있음 여부 0 Empty
float tempture = 0; // 온도

volatile boolean IhavSel = 0; // Master 선택 여부
volatile boolean complet_JOB = false; // 루틴 확인용
boolean LOCK_OFF = true; // SPI Master -> UNLOCK order

int MENU = 1; // 작업 선택

int soundinside(){
  digitalWrite(PIN_Trig, LOW);   delayMicroseconds(2);
  digitalWrite(PIN_Trig, HIGH);  delayMicroseconds(10);
  digitalWrite(PIN_Trig, LOW);
  T = pulseIn(PIN_Echo, HIGH);
  L = T / 58.82;

  if (L >= rangeMin && L <= rangeMax) 
    return 1; // 있으면 1
  else 
    return 0; // 없으면 0
} // for Ultrasound Funtion 

int LM35(){
  return (analogRead(PIN_LM35) * 5.0 * 100.0) / 1024.0;
}

void sendUART(const byte what){
  mySerial.write(what);
  delayMicroseconds(20);
  return;
} // for UART Commiuncation

void Relayinit() {
  MENU = 1;
  IhavSel = 0;
  digitalWrite(PIN_LOCK, HIGH);
  digitalWrite(PIN_FAN, HIGH);
  LOCK_OFF = true;
  complet_JOB = false;
  itsLOCK = true;
  sendUART(5);
} // for Cabinet init

void setup() {
  Serial.begin(9600); 
  mySerial.begin(9600); // UART Init 

  // SPI SPCR Config Set
  SPCR |= _BV(SPIE);
  SPCR |= _BV(SPE);
  SPCR &= ~_BV(MSTR);

  // 솔레노이드 & 팬
  pinMode(PIN_LOCK, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  digitalWrite(PIN_LOCK, HIGH); // LOW == unlock : HIGH == lock
  digitalWrite(PIN_FAN, HIGH); // LOW == Working : HIGH == Stop 

  // 마그네틱
  pinMode(PIN_MAG, INPUT_PULLUP); // 내부 풀업 저항 사용

  // 초음파
  pinMode(PIN_Trig, OUTPUT);
  pinMode(PIN_Echo, INPUT);
}

void loop() {
  delay(1000);

  if(IhavSel == 1){ // 0-0. 선택됨

    // 1. Update Status
    // 1-1. itsInside? (초음파)
    Serial.print("InThere? : ");
    inside = soundinside();
    Serial.println(inside); // Monitering

    // 1-2. Mgnetic (OPEN 1 : CLOSE 0)
    itsOPEN = digitalRead(PIN_MAG);
    Serial.print("PIN_MAG : ");
    Serial.println(itsOPEN); // Monitoring
    delay(100);

    // 2. MENU Sequnece
    if (MENU == 1){ // 2-1. 초기, 넣는 과정
      Serial.println("1. its Sequnece ONE");
      
      // 2-1-1. UART Comm (Green Light)
      sendUART(3);

      // 2-1-2. Unlock 
      if(itsLOCK == 1){
        itsLOCK = 0;
        digitalWrite(PIN_LOCK, LOW); // 솔레노이드 열림.
      }

      // 2-1-3. Ultrasound Init
      Serial.print("InThere? : ");
      inside = soundinside(); // 초음파 초기화
      Serial.println(inside); // Monitering

      // 2-1-4. MENU 2 Status Condition
      if(inside == 1 && itsOPEN == 0){ // 안에 넣고 & 문이 닫히면?
        itsLOCK = 1; //잠김
        digitalWrite(PIN_LOCK, HIGH);
        MENU = 2; // Sequnece 2로 진행
      }

    } 

    else if (MENU == 2) { // 2-2. 넣고 빼는 과정
      Serial.println("2. its Sequnece TWO");

      // 2-2-1. UART Comm (Green Light)
      sendUART(4);

      // 2-2-2. LM35 Tempture
      tempture = LM35();
      Serial.print("Temp : ");
      Serial.println(tempture);
      delay(1000);

      // EX) SPI Trigger
      if(LOCK_OFF == true) { // SPI 열라고 하면?
        itsLOCK = 0; // 잠금 해제
        digitalWrite(PIN_LOCK, LOW);
      }

      // 2-2-3. 초음파 센서 Init
      Serial.print("InThere? : ");
      inside = soundinside();
      Serial.println(inside);

      // EX) Exit Condition (잠기지 않음, 안이 비었고, 문 닫힘)
      if(itsLOCK == 0 && inside == 0 && itsOPEN == 0) {
        complet_JOB = true;
        MENU = 1;
      }

      // Etc.) FAN control
      if (tempture >= 23){ // 23도 이상이면
        digitalWrite(PIN_FAN, LOW); // 0이면 작동
      } else {
        digitalWrite(PIN_FAN, HIGH); // 1이면 정지
      }
    }

    // Exit AND Init
    if(complet_JOB) { // 일이 끝났으면
      Relayinit();
    }

  } else {  // 0-1. 선택안됨
    Serial.println("NOT Selet");
  }

}

ISR(SPI_STC_vect){
  byte c = SPDR;

  if (c == 's') {
    Serial.println("SPI"); // SPI Alarm

    IhavSel = 1;

    LOCK_OFF = !LOCK_OFF;

    return;
  }
}