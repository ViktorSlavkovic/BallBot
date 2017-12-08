#include <string.h>

#include "TimerOne.h"

const int kSendDataPins[] = {7, 8, /*9,*/ 10};
const int kSendIntPin = 6;
const int kBuzzPin = 3;
const int kMpuI2cIntPin = 2;
const int kMuxSelectPins[] = {4, 5};
const int kLedPins[] = {A2, A3, A1};

#define kStaticIp "192.168.1.50" 
#define kNetSSID "SSID" 
#define kNetPass "PASS"
#define kDestIp "192.168.1.3" 
#define kSrcDestPort "12345"

class Parallel {
 public:
  static void SendByte(uint8_t x) {
    SendLower3b(x);
    x >>= 3;
    SendLower3b(x);
    x >>= 3;
    SendLower3b(x);
  }
  
 private:
  static void SendLower3b(uint8_t xl) {
    uint8_t mask = 1;
    for (int i = 0; i < 3; i++) {
      digitalWrite(kSendDataPins[i], xl & mask);
      mask <<= 1;
    }
    digitalWrite(kSendIntPin, HIGH);
    delay(1);
    digitalWrite(kSendIntPin, LOW);
    delay(1);
  }
};

class WiFi {
 public:
  enum Command {
    LEFT, RIGHT, MOVE, NONE
  };

  static const int kNumCommands = 3;

  static const char* CommandMsgs[kNumCommands] = {
    "+IPD,5:LEFT",
    "+IPD,6:RIGHT",
    "+IPD,5:MOVE"
  };
  
  static Init() {
    Serial.println("AT+RST");
    Serial.println("ATE0");
    Serial.println("AT+SLEEP=0");
    Serial.println("AT+CWMODE_CUR=1");
    Serial.println("AT+CWAUTOCONN=0");
    Serial.println("AT+CWDHCP_CUR=1,0");
    Serial.println("AT+CIPSTA=\"" kStaticIp "\"");
    Serial.println("AT+CWJAP_CUR=\"" kNetSSID "\",\"" kNetPass "\"");
    Serial.println("AT+CIPMUX=0");
    Serial.println("AT+CIPMODE=0");
    Serial.println("AT+CIPSTART=\"UDP\",\"" kDestIp "\"," kSrcDestPort "," kSrcDestPort);
    FlushRx();
  }
  
  static void Send(const char* msg) {
    int n = strlen(msg);
    Serial.print("AT+CIPSEND=");
    Serial.println(String(n).c_str());
    Serial.print(msg);
    Serial.println();
  }

  // Every second line will probably be empty, because ESP
  // probably always returns both /13 and /10, but we don't
  // really care.
  static void ReadLineToBuffer() {
    int pos = 0;
    while (true) {
      if (!Serial.available()) continue;
      char c = Serial.read();
      if (c == '\n' || c =='\r') {
        break;
      }
      buffer[pos++] = c;
    }
    buffer[pos++] = 0;
  } 

  static Command ParseCommand() {
    for (int i = 0; i < kNumCommands; i++) {
      if (strcmp(buffer, CommandMsgs[i]) == 0) {
        return (Command)i;  
      }
    }
    return NONE;
  }
  
 private:
  static char buffer[1024];
  
  static void FlushRx() {
    while (Serial.available()) {
      Serial.read();  
    }  
  }
};

char WiFi::buffer[1024];

class MPU {
 public:
  static void Fetch() {}
  static void Filter() {} 
  static void GetZAngles(double& a, double& b, double& c) {
    a = random(9000) * 0.01;
    b = random(9000) * 0.01;
    c = random(9000) * 0.01;
  }
};

void TimerCallback() {
  MPU::Fetch();
  MPU::Filter();
  double az, bz, cz;
  MPU::GetZAngles(az, bz, cz);
  // Calculate motor speeds.
  // Use Parallel to send the speeds to Arduino 2.
  //WiFi::Send((String(az, 2) + " : " + String(bz, 2) + " : " + String(cz, 2)).c_str()); 
}

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(kSendDataPins[i], OUTPUT);
    digitalWrite(kSendDataPins[i], LOW);
  }
  pinMode(kSendIntPin, OUTPUT);
  digitalWrite(kSendIntPin, LOW);
  pinMode(kBuzzPin, OUTPUT);
  digitalWrite(kBuzzPin, LOW);

  for (int i = 0; i < 3; i++) {
    pinMode(kLedPins[i], OUTPUT);
    analogWrite(kLedPins[i], 255);
  }
  
  Serial.begin(115200);

  delay(1000);

  WiFi::Init();

  delay(5000);

  randomSeed(analogRead(0));
  
  Timer1.initialize(500000);
  Timer1.pwm(9, 512);
  Timer1.attachInterrupt(TimerCallback);
}

void loop() {
  WiFi::ReadLineToBuffer();
  switch (WiFi::ParseCommand()) {
    LEFT: {
      WiFi::Send("LEFT");
      break;
    }
    RIGHT: {
      WiFi::Send("RIGHT");
      break;
    }
    MOVE: {
      WiFi::Send("MOVE");
      break;
    }
  }
}
