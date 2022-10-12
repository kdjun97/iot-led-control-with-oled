#include "DHTesp.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED 관련 정의
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// GPIO 핀 정의
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

// 핀 맵핑 정의
#define LIGHT_PIN A0
#define LED_PIN D0
#define DHTPIN D3
#define RELAY1_PIN D4 // D4로 맵핑 변경

// 릴레이 상태 정의
#define RELAY_OFF HIGH // HIGH 신호이면 릴레이 동작하지 않음
#define RELAY_ON LOW // LOW 신호이면 릴레이 동작함

// 각종 전역 변수
bool check = true;
int light_val;
int relay_state = RELAY_OFF; // 릴레이 OFF
const int threshold = 500;
bool special_condition = false;
bool prior_state;
float humidity, temperature;
char str_temperature[40], str_humidity[40];
unsigned long time_snapshot1; // 시간을 위한 변수
unsigned long time_snapshot2; // USB LED 시간을 위한 변수

DHTesp dht;

void setup() {
  Serial.begin(9600);
  pinMode(D0, OUTPUT);
  pinMode(LIGHT_PIN, INPUT);
  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW); //LED off
  pinMode (RELAY1_PIN, OUTPUT); // 릴레이 signal 단자
  digitalWrite (RELAY1_PIN, RELAY_OFF); // 릴레이 OFF
  dht.setup(DHTPIN, DHTesp::DHT22); // DHTPIN(D3)에 맵핑 

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 allocation successed"));
  fetch_light();
  prior_state = (light_val > threshold) ? true : false;
  time_snapshot1 = millis();
  time_snapshot2 = 0;    
}

void loop() {
  if (time_snapshot1 + 1000 <= millis()) {
    led_on();
    fetch_dht22_info();
    fetch_light();
    control_usbled(threshold);
    write_dht22_info_to_display();
    time_snapshot1 = millis(); 
  }
}

// LED 깜빡거림.
void led_on() {
  check ? digitalWrite(D0, HIGH) : digitalWrite(D0, LOW);  
  check = !check;
}

// 온습도
void fetch_dht22_info() {
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();

  if (!isnan(humidity) && !isnan(temperature)) { // !(not a number), humidity와 temperature가 숫자 값이라면
    Serial.print("Temp:");
    Serial.print(temperature);
    Serial.print("\tHumidity:");
    Serial.println(humidity);
  }
  else
    Serial.println("Fetch DHT22 Data Error!");
}

void write_dht22_info_to_display() {
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("T:"));
  display.print(temperature);
  display.println(F("'C"));
  
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.print(F("H:"));
  display.print(humidity);
  display.println(F("%"));
  
  display.display();
}

// 조도 센서
void fetch_light() {
  light_val = analogRead(LIGHT_PIN);
  Serial.print("Light intensity = ");
  Serial.println(light_val);
}

// USB LED 컨트롤 함수
void control_usbled(int t) {
  if (special_condition && time_snapshot2 + 10000 <= millis()) {
    special_condition = false;
    relay_state = RELAY_OFF;
  }
  else {
    if (prior_state && light_val < t) { // 밝은 -> 어두운 상태
      prior_state = false;
      special_condition = true; // 특수 상태 값 체크 
      time_snapshot2 = millis(); // 10초 시간 재기 위한 부분
      relay_state = RELAY_ON;
    }
    else if (prior_state && light_val > t) { // 밝은 -> 밝은 상태
      prior_state = true;
    }
    else if (!prior_state && light_val < t) { // 어두운 -> 어두운 상태
        prior_state = false;
    }
    else if (!prior_state && light_val > t) { // 어두운 -> 밝은
      prior_state = true;
    }
  }  
  digitalWrite(RELAY1_PIN, relay_state);  
}
