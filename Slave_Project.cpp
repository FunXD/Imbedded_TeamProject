#include <Arduino.h>

const int PIN_LOCK = 5;
const int PIN_FAN = 4;
const int PIN_MAG = 9;
boolean status_FAN = false;
boolean status_LOCK = false;

const int pinTrig = 8;
const int pinEcho = 7;
int rangeMax = 10;
int rangeMin = 0;
long T, L;

volatile byte command = 0;
boolean IhavSel = 0;
boolean complet_JOB = false; // 루틴 확인용

void setup() {
  Serial.begin(9600); // Monitoring

  SPCR |= _BV(SPIE);
  SPCR |= _BV(SPE);
  SPCR &= ~_BV(MSTR);

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
  if(IhavSel){
    // 1. 초음파 계산 
    digitalWrite(pinTrig, LOW); 
    delayMicroseconds(2);
    digitalWrite(pinTrig, HIGH); 
    delayMicroseconds(10);
    digitalWrite(pinTrig, LOW);

    T = pulseIn(pinEcho, HIGH);
    L = T/58.82;
    Serial.println(L); // Moniter
    delay(1000);
    
    // 2. Mgnetic
    if (digitalRead(PIN_MAG)){
      Serial.print("PIN_MAG : ");
      Serial.println(PIN_MAG);

    }

    // FAN Control
    if (L >= rangeMax || L <= rangeMin) 
      digitalWrite(PIN_FAN, HIGH);
    else 
      digitalWrite(PIN_FAN, LOW);

    // LOCK Control
    if(status_LOCK){
      digitalWrite(PIN_LOCK, LOW);
    }

    // 1. 종료 : 끝났으면?
    if(complet_JOB) { // 일이 끝났으면
  
      pinMode(MISO, OUTPUT); // MISO ON

    }
  }
}

ISR(SPI_STC_vect){
  byte c = SPDR; // BUFFER
  if (c == 's'){
    IhavSel = 1;
    return;
  }

  switch(command){
    case 0 : 
      command = c;
      SPDR = 0;
      break;

    case 'c': 
      SPDR = complet_JOB;
      break;
  }
  
}