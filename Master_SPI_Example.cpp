// - SPI Master -
// 1. Condition
//    1. 연결 가능한 상자는 일단 3개로
// 2. instruction
//    1. c = 보관함 작업 끝
//    2. s = 마스터가 보관함 선택 & 보관함 작업 시작


#include <Arduino.h>
#include <SPI.h>

typedef struct{
  int SS_Address; // 주소
  int its_Empty;  // 비어있는지 여부
  // ...
  // 더 추가 하고 싶은 정보
} BOX;

BOX box[3];  //점거중인 모듈 3개, 1 Empty : 0 Full

int who_Selet = 0; // 누구 선택?

void setup() {
  Serial.begin(9600); // Monitoring
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  // ...

  int box_index = 0;
  for(int pin = 8; pin <= 10; pin++){ 
    box[box_index].SS_Address = pin;
    box[box_index].its_Empty = 1;
    
    pinMode(pin, OUTPUT); 
    digitalWrite(pin, HIGH);
    
    box_index++;
  } // init Slave
}

byte transferAndWait(const byte what){
  byte a = SPI.transfer(what);
  delayMicroseconds(20);
  return a;
}

void loop(){
  int who_Selet = 0;

  // ...
  // ...

  // Status Update 상태 업데이트
  for (int i = 0; i < 3; i++){
    if(box[i].its_Empty == 0){ // 차있는 보관함만 송신
      byte s;
      digitalWrite(box[i].SS_Address, LOW);

      transferAndWait('c'); // Complete? Working?
      transferAndWait(0); // buffer
      s = transferAndWait(0); // sending

      if(s == 1){ // 끝났으면
        box[i].its_Empty = 1;
      }

      digitalWrite(box[i].SS_Address, LOW);
    }
  }

  for(int i = 0; i < 3; i++){
    if(box[i].its_Empty == 1){ // 비어 있으면?
      who_Selet = box[i].SS_Address; // 주소 선택
      box[i].its_Empty = 0; // FULL
      break;
    }
  } // 기능 : who Select?

  if(who_Selet == 0) {
      Serial.println("ALL BOXES FULL - DO NOT USE");
      delay(1000);
      return; // 루프 재시작
  } // else : 꽉차 있다면
  delay(5000);

  // SPI 통신
  if(who_Selet > 0){
    Serial.print("SPI Output to Pin: ");
    Serial.println(who_Selet); // 선택된 주소 모니터링

    digitalWrite(who_Selet, LOW); // Start
    SPI.transfer('s'); // 너 선택된거야
    Serial.println("SEND COM"); // 전송 완료

    // ...
    // ...

    digitalWrite(who_Selet, HIGH); // END
  }

}