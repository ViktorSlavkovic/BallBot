#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

extern "C" {
#include "user_interface.h"
}

const char* kWiFiSSID = "VPS";
const char* kWiFiPassword = "milorad__senic";
const int   kUdpServerPort = 12345;
IPAddress ip(192, 168, 1, 253);
IPAddress gateway(192, 168, 1, 1); 
IPAddress subnet(255, 255, 255, 0); 
WiFiUDP udp_server;
const int kBufferSize = 1024;
char buffer[kBufferSize];

void setup() {
  // Motor control pins init.
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D3, LOW);
  digitalWrite(D4, HIGH);
  // Connect to WiFi.
  WiFi.config(ip, gateway, subnet); 
  WiFi.begin(kWiFiSSID, kWiFiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Run UDP server.
  while (!udp_server.begin(kUdpServerPort)) {}
}

// 0 - stop
// 1 - up
// 2 - down
void MotorControl(int action) {
  switch(action) {
    case 1: {
      digitalWrite(D3, HIGH);
      digitalWrite(D4, LOW);
      break;
    }
    case 2: {
      digitalWrite(D3, LOW);
      digitalWrite(D4, HIGH);
      break;
    }
    default: {
      digitalWrite(D3, LOW);
      digitalWrite(D4, LOW);
      break;
    }
  }
}



void loop() {
  int n;
  if ((n = udp_server.parsePacket()) > 0) {
    udp_server.read(buffer, kBufferSize);
    if (memcmp(buffer, "UP", 2) == 0) {
      MotorControl(1);
      delay(1000);
      MotorControl(0);
    } else if (memcmp(buffer, "DOWN", 4) == 0) {
      MotorControl(2);
      delay(1000);
      MotorControl(0);
    }
  }   
}
