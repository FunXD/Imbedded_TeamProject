#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h> // 추가 라이브러리 다운로드 필요!

const int PIN_NEOPIXEL  = 6;    // 네오픽셀 DIN

SoftwareSerial mySerial(8, 9); 

const int NUM_PIXELS = 16;      
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

bool isSystemActive = false; 
volatile int LED_color; // ++ 

// 색상 변경 함수
void setNeoPixelColor(int r, int g, int b) {
  for(int i=0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  pixels.begin();
  pixels.setBrightness(50); // 밝기 조절 함수 (너무 밝아서 줄이는...)
  pixels.show();
}

void loop() {
  if (mySerial.available()) {
    byte cmd = mySerial.read();
    Serial.println(cmd);
    if (cmd == 3) { // 3 (Green 배달기사 유도등) or 4 (Blue 사용자 안내등)
      isSystemActive = 1;
      LED_color = cmd;
    } else if (cmd == 4) { // Blue LED == 사용자 안내용
      isSystemActive = 1;
      LED_color = cmd;
    } else if (cmd == 5) { // Off Condition
      isSystemActive = 0;
    }
  }
  
  if (isSystemActive) {
    if (LED_color == 3) { //
      setNeoPixelColor(0, 255, 0);   
    } else if (LED_color == 4) { // 물건 없음 -> 파란색
      setNeoPixelColor(0, 0, 255); 
    }
  } else {
    setNeoPixelColor(0, 0, 0); // 전원 off
  }
  
  delay(10); 
}
