#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

const int PIN_TRIG      = 4;
const int PIN_ECHO      = 3;
const int PIN_NEOPIXEL  = 6;

SoftwareSerial mySerial(8, 9); 

const int NUM_PIXELS = 16;      
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

const int BOX_HEIGHT = 29; // 박스 임시 높이
const int ERROR_MARGIN = 1; // 박스 높이 오차 범위 

bool isSystemActive = false; // 3을 보내기 전 꺼져있음

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  
  pixels.begin();
  pixels.setBrightness(29); 
  pixels.show(); // 초기 상태: 꺼짐

  Serial.begin(9600);   
  mySerial.begin(9600); 
  Serial.println("System Started (Wait for '3')");
}

void loop() {
  if (mySerial.available()) {
    char cmd = mySerial.read(); // 대기
    
    if (cmd == '3') { // 만약 3 명령을 받았을 경우
      isSystemActive = true; 
    }
    //
  }
  
  long duration, distance;
  digitalWrite(PIN_TRIG, LOW); delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  duration = pulseIn(PIN_ECHO, HIGH);
  distance = duration * 0.034 / 2;


  static unsigned long lastSendTime = 0; // 거리값 전송
  if (millis() - lastSendTime > 500) { 
    int sendDist = (distance > 255) ? 255 : distance;
    mySerial.write((byte)sendDist); 
    lastSendTime = millis();
  }

  if (isSystemActive == true) { // 거리를 통한 LED 색상 변경 
    if (distance >= (BOX_HEIGHT - ERROR_MARGIN)) {
      setNeoPixelColor(0, 0, 255); // 물건 없을 때
    } 
    else {
      setNeoPixelColor(255, 0, 0); // 물건 있을 때
    }
  } 
  else {
    setNeoPixelColor(0, 0, 0); // 시스템 비활성화
  }
  
  delay(50); 
}

void setNeoPixelColor(int r, int g, int b) { // 색상 변경 함수
  for(int i=0; i<NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}
