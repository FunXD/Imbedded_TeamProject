#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h> // 추가 라이브러리 다운로드 필요!

const int PIN_TRIG      = 4;
const int PIN_ECHO      = 3;
const int PIN_NEOPIXEL  = 6;    // 네오픽셀 DIN

SoftwareSerial mySerial(8, 9); 

const int NUM_PIXELS = 16;      
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

bool isSystemActive = false; 

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  
  pixels.begin();
  pixels.setBrightness(50); // 밝기 조절 함수 (너무 밝아서 줄이는...)
  pixels.show();
}

void loop() {
  if (mySerial.available()) {
    char cmd = mySerial.read();
    if (cmd == '3') {
      isSystemActive = true; 
    }
  }
  
  if (isSystemActive) {
    if (inside == 1) { // 물건 있음 -> 빨간색
      setNeoPixelColor(255, 0, 0);   
    } 
    else { // 물건 없음 -> 파란색
      setNeoPixelColor(0, 0, 255); 
    }
  } 
  else {
    setNeoPixelColor(0, 0, 0); // 전원 off
  }
  delay(50); 
}

// 색상 변경 함수
void setNeoPixelColor(int r, int g, int b) {
  for(int i=0; i<NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}
