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
int rangeMax = 10; // 20cm 감지
int rangeMin = 0;
long T, L;

int inside = 0; // 안에 있음 여부 0 Empty
float tempture = 0; // 온도

volatile boolean IhavSel = 0;
volatile boolean complet_JOB = false; // 루틴 확인용
boolean LOCK_OFF = true; // SPI 명령. 

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
} // for ultrasound

void Relayinit() {
  digitalWrite(PIN_LOCK, HIGH);
  digitalWrite(PIN_FAN, HIGH);
  LOCK_OFF = true;
  complet_JOB = false;
  itsLOCK = true;
} // for init

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
  digitalWrite(PIN_LOCK, HIGH); // LOW == unlock : HIGH == lock
  digitalWrite(PIN_FAN, HIGH);

  // 마그네틱
  pinMode(PIN_MAG, INPUT_PULLUP);

  // 초음파
  pinMode(PIN_Trig, OUTPUT);
  pinMode(PIN_Echo, INPUT);
}

void loop() {
  delay(1000);

  if(IhavSel == 1){ // 선택됨

    // 1-1-1. itsInside? (초음파)
    Serial.print("InThere? : ");
    inside = soundinside();
    Serial.println(inside); // Monitering

    // 1-2. Mgnetic (OPEN 1 : CLOSE 0)
    itsOPEN = digitalRead(PIN_MAG);
    Serial.print("PIN_MAG : ");
    Serial.println(itsOPEN); // Monitoring
    delay(100);

    // 선택
    if (MENU == 1){ // 초기 넣는 과정
      Serial.println("Sequnce 1");
      
      if(itsLOCK == 1){
        itsLOCK = 0;
        digitalWrite(PIN_LOCK, LOW); // 솔레노이드 열림.
      }

      Serial.print("InThere? : ");
      inside = soundinside(); // 초음파 초기화
      Serial.println(inside); // Monitering

      if(inside == 1 && itsOPEN == 0){ // 안에 넣고 & 문이 닫히면?
        itsLOCK = 1; //잠김
        digitalWrite(PIN_LOCK, HIGH);
        MENU = 2; // Sequnce 2로 진행
      }

    } 

    else if (MENU == 2) { // 넣고 빼는 과정
      Serial.println("its Sequnce 2");

      // 1-3. LM35 Tempture
      tempture = (analogRead(PIN_LM35) * 5.0 * 100.0) / 1024.0;
      Serial.print("Temp : ");
      Serial.println(tempture);
      delay(1000);

      // SPI Trigger
      if(LOCK_OFF == true) { // SPI 열라고 하면?
        itsLOCK = 0; // 잠금 해제
        digitalWrite(PIN_LOCK, LOW);
      }

      // 1-2 초음파 센서
      Serial.print("InThere? : ");
      inside = soundinside();
      Serial.println(inside);

      // 끝나는 조건 (잠기지 않았고, 안에가 비었고, )
      if(itsLOCK == 0 && inside == 0 && itsOPEN == 0) {
        complet_JOB = true;
        MENU = 1;
      }

      // etc.) FAN control
      if (tempture >= 23){ // 23 이상이면
        digitalWrite(PIN_FAN, LOW); // 0이면 작동
      } else {
        digitalWrite(PIN_FAN, HIGH); // 1이면 정지
      }
    }

    // 1. 종료 : 끝났으면?
    if(complet_JOB) { // 일이 끝났으면
      MENU = 1;
      IhavSel = 0;
      Relayinit();
    }

  } else {
    Serial.println("NOT Selet");
  }

}

ISR(SPI_STC_vect){
  byte c = SPDR;

  if (c == 's') {
    Serial.println("SPI");

    IhavSel = 1;

    LOCK_OFF = !LOCK_OFF;

    return;
  }
  
}