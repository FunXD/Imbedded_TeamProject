#include <LiquidCrystal_I2C.h>      //라이브러리 다운로드 필요
#include <Keypad.h>                 //이것도
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); //2004 LCD SCL,SDA,GND,5V
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'},
};
String room ="";
int CS = 13;                        //CursorStart 커서시작위치 
byte rowPins[ROWS] = {8,7,6,5};     //3x4버튼 2~8번핀
byte colPins[COLS] = {4,3,2};
bool confirmMode = false;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void MainScreen(){                  //메인화면
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("Welcome To");
  lcd.setCursor(0,1);
  lcd.print("SMART BAEDAL SERVICE");
  lcd.setCursor(0,2);
  lcd.print("Enter your");
  lcd.setCursor(0,3);
  lcd.print("room number:");
  CS = 13;
  room = "";
  confirmMode = false;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  MainScreen();
}

void loop() {
  char key = keypad.getKey();
  if (confirmMode){                 //입력 확인 모드일때
    if (key == '#'){                //사물함 열림(추가예정)
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("locker 3 open");
      delay(1500);
      MainScreen();
    }
    else if (key=='*'){             //메인화면 복귀
      MainScreen();
    }
    return;
  }
  if (key >= '0' && key<='9'){      //숫자입력
    if (room.length() <= 3){
      lcd.setCursor(CS,3);
      lcd.print(key);
      Serial.println(key);
      room += key;
      CS++;
    }
  }
  else if (key =='*'){              //지우기
    if (CS > 13){
      CS--;
      room.remove(room.length() -1);
      lcd.setCursor(CS,3);
      lcd.print(" ");
      lcd.setCursor(CS,3);
    }
  }
  else if (key == '#'){             //엔터
    confirmMode = true;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("is " + room + " correct?");
    lcd.setCursor(5,2);
    lcd.print("no:* yes:#");
  }
}