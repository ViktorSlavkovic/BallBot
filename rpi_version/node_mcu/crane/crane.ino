#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

extern "C" {
#include "user_interface.h"
}

const int kInterruptPin = D8;
const char* kWiFiSSID = "VPS";
const char* kWiFiPassword = "milorad__senic";
const int   kUdpServerPort = 12345;
IPAddress ip(192, 168, 2, 253);
IPAddress gateway(192, 168, 2, 1); 
IPAddress subnet(255, 255, 255, 0); 
WiFiUDP udp_server;
const int kBufferSize = 1024;
char buffer[kBufferSize];

const int kIntArduinoPin = D4;
const int kIntData0ArduinoPin = D3;
const int kIntData1ArduinoPin = D2;
const int kIntData2ArduinoPin = D1;

volatile bool going_up = false;

// 0 - stop
// 1 - up
// 2 - down
void MotorControl(int action) {
  going_up = false;
  switch(action) {
    case 1: {
      going_up = true;
      digitalWrite(D5, LOW);
      digitalWrite(D6, HIGH);
      break;
    }
    case 2: {
      digitalWrite(D5, HIGH);
      digitalWrite(D6, LOW);
      break;
    }
    default: {
      digitalWrite(D5, HIGH);
      digitalWrite(D6, HIGH);
      break;
    }
  }
}

void Interrupt() {
  if (going_up) {
    MotorControl(0);
  }
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(kInterruptPin), Interrupt, HIGH);
  // Motor control pins init.
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  // Arduino comm pins init.
  pinMode(kIntArduinoPin, OUTPUT);
  digitalWrite(kIntArduinoPin, LOW);
  pinMode(kIntData0ArduinoPin, OUTPUT);
  pinMode(kIntData1ArduinoPin, OUTPUT);
  pinMode(kIntData2ArduinoPin, OUTPUT);
  // Connect to WiFi.
  WiFi.config(ip, gateway, subnet); 
  WiFi.begin(kWiFiSSID, kWiFiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Run UDP server.
  while (!udp_server.begin(kUdpServerPort)) {}

  //Serial.begin(115200);
  //Serial.println("RUNNING...");
}

int ReadIntParam(char* p) {
  int res = 0;
  while (isdigit(*p)) {
    res *= 10;
    res += *p - '0';
    p++;
  }
  return res;
}


const long kThrottleAnyThreshold = 1000;
const long kThrottleSameThreshold = 2000;
long ThrottleThresholdDelay = 0;

bool ThrottleRequests(int req_type) {
  static int prev_req_type = -1;
  static long prev_req_time = 0;
  long now = millis();
  bool res = false;
  res = res || (prev_req_type == req_type && now - prev_req_time < kThrottleSameThreshold + ThrottleThresholdDelay);
  res = res || (prev_req_type != req_type && now - prev_req_time < kThrottleAnyThreshold + ThrottleThresholdDelay);
  prev_req_type = req_type;
  prev_req_time = now;
  //Serial.println(res ? "Throttled" : "Not Throttled");
  return res;
}

void ParseMessage() {
  //Serial.println(buffer);
  
  int req_type_cnt = 0;
  
  // DOWN@DURATION@
  if (req_type_cnt++, memcmp(buffer, "DOWN", 4) == 0) {
    if (ThrottleRequests(req_type_cnt)) return;
    int duration = ReadIntParam(buffer + 5);
    MotorControl(2);
    delay(duration);
    MotorControl(0);
    ThrottleThresholdDelay = duration;
    return;
  }
  
  // UP@DURATION@
  if (req_type_cnt++, memcmp(buffer, "UP", 2) == 0) {
    if (ThrottleRequests(req_type_cnt)) return;
    
    int duration = ReadIntParam(buffer + 3);
    MotorControl(1);
    delay(duration);
    MotorControl(0);
    ThrottleThresholdDelay = duration;
    return;
  }

  // TREADMILL@ACTION@
  if (req_type_cnt++, memcmp(buffer, "TREADMILL", 9) == 0) {
    if (ThrottleRequests(req_type_cnt)) return;
    int action = ReadIntParam(buffer + 10);
    digitalWrite(kIntData0ArduinoPin, action ? HIGH : LOW);
    digitalWrite(kIntData1ArduinoPin, action ? HIGH : LOW);
    digitalWrite(kIntData2ArduinoPin, action ? HIGH : LOW);
    delay(500);
    digitalWrite(kIntArduinoPin, HIGH);
    delay(100);
    digitalWrite(kIntArduinoPin, LOW);
    ThrottleThresholdDelay = 0;
    return;
  }

  //Serial.println("UNKNOWN");
}

void loop() {
  int n;
  if ((n = udp_server.parsePacket()) > 0) {
    udp_server.read(buffer, kBufferSize);
    ParseMessage();
  }   
}
