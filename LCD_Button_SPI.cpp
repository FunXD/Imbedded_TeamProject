#include <LiquidCrystal_I2C.h>    //라이브러리 다운로드 필요
#include <Keypad.h>               //라이브러리 다운로드 필요
#include <Wire.h>
#include <Arduino.h>
#include <SPI.h>


LiquidCrystal_I2C lcd(0x27, 20, 4); // 2004 LCD SCL,SDA,GND,5V
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'},
};

byte rowPins[ROWS] = {8,7,6,5};
byte colPins[COLS] = {4,3,2};

String room = "";
int CS = 13;                        // CursorStart 입력/삭제시 커서 위치

enum Mode { MAIN, ADD, FIND };       //메인화면, 맡기기, 찾기  
Mode currentMode = MAIN;
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String roomList[10];                // 동호수 리스트 (최대 10개)
bool exists, found;                 //동호수 존재여부, 찾아갔는지 여부
int roomCount = 0;

//===============================SPI=====================================

const int SS_PIN = 10;            // SPI 10번 핀입니다!!
int box_isEmpty = 1;

byte transferAndWait(const byte what) {
  byte a = SPI.transfer(what);
  delayMicroseconds(20);
  return a;
}

void openLocker() {
  digitalWrite(SS_PIN, LOW);
  transferAndWait('s');               // 열림 신호
  digitalWrite(SS_PIN, HIGH);
  box_isEmpty = 0;
}

//========================================================================

void MainScreen() {                 // 메인화면
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("Welcome To");
  lcd.setCursor(0,1);
  lcd.print("SMART BAEDAL SERVICE");
  lcd.setCursor(0,2);
  lcd.print("Select Option ");
  lcd.setCursor(0,3);
  lcd.print("*: Add   #: Find");
  room = "";
  CS = 0;
  currentMode = MAIN;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  MainScreen();

  SPI.begin();                        //SPI
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);
}

void loop() {
  char key = keypad.getKey();

  if (!key) return;                  // 키가 안눌리면 무시

  switch(currentMode) {
    
    case MAIN:
      if (key == '*') {             //보관하기  
        room = "";
        CS = 0;
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Enter room to add:");
        lcd.setCursor(0,2);
        currentMode = ADD;
      }
      else if (key == '#') {        // 찾기
        room = "";
        CS = 0;
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Enter room to find:");
        lcd.setCursor(0,1);
        currentMode = FIND;
      }
      break;

    case ADD:                   // 넣기 모드와 찾기 모드일때 동호수 입력
    case FIND:
      if (key >= '0' && key <= '9') {
        if (room.length() <= 3) {
          lcd.setCursor(CS,2);
          lcd.print(key);
          room += key;
          CS++;
        }
      } else if (key == '*') {              // 입력 숫자 지우기
        if (CS > 0) {
          CS--;
          room.remove(room.length() - 1);
          lcd.setCursor(CS,2);
          lcd.print(" ");
          lcd.setCursor(CS,2);
        }
      } else if (key == '#') {              // 엔터
        if (currentMode == ADD) {     
          exists = false;
          for (int i=0; i<roomCount; i++){
            if (roomList[i] == room){
              exists = true;                //방이 이미 존재함!
            }
          }
          if (!exists && roomCount < 10) {  //없을경우 추가
            roomList[roomCount++] = room;
          }
          lcd.clear();
          lcd.setCursor(2,1);
          lcd.print("locker 3 Opened!");
          openLocker();                     //보관함 열기
          delay(1500);
          MainScreen();
        }
        else if (currentMode == FIND) {
          found = false;
          int idx = -1;
          for (int i=0; i<roomCount; i++){
            if (roomList[i] == room) {
              found = true;
              idx = i;
              break;
            }
          }
          lcd.clear();
          if (found) {                    //음식을 찾아감
            lcd.setCursor(2,1);
            lcd.print("locker 3 Opened!");
            openLocker();                 //보관함 열기
            for (int i=idx; i<roomCount-1; i++) {
              roomList[i] = roomList[i+1];
            }
            roomCount--;
          } else {
            lcd.setCursor(0,1);
            lcd.print("Room not found");
          }
          delay(1500);
          MainScreen();
        }
      }
      break;

    default:
      MainScreen();
      break;
  }
//=====================SPI============================
  if (!box_isEmpty) {
    digitalWrite(SS_PIN, LOW);
    transferAndWait('c'); // 상태 확인
    transferAndWait(0);
    byte status = transferAndWait(0);
    digitalWrite(SS_PIN, HIGH);
    if (status == 1) {
      box_isEmpty = 1;
    }
  }
}
