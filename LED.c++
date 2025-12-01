#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

const int PIN_TRIG      = 4;
const int PIN_ECHO      = 3;
const int PIN_NEOPIXEL  = 6;    // 네오픽셀 DIN

SoftwareSerial mySerial(8, 9); 

const int NUM_PIXELS = 16;      
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

const int BOX_HEIGHT = 29;
const int ERROR_MARGIN = 1;

bool isSystemActive = false; 

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  
  pixels.begin();
  pixels.setBrightness(50); 
  pixels.show();

  Serial.begin(9600);   
  mySerial.begin(9600); 
  Serial.println("Sensor Board Ready");
}

void loop() {
  if (mySerial.available()) {
    char cmd = mySerial.read();
    if (cmd == '3') {
      isSystemActive = true; 
    }
  }
  
  if (distance < (BOX_HEIGHT - ERROR_MARGIN)) {
    inside = 1;
  } else {
    inside = 0;
  }

  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime > 500) { 
    int sendDist = (distance > 255) ? 255 : distance;
    mySerial.write((byte)sendDist);
    lastSendTime = millis();
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
