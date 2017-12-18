#include <string.h>

#include "TimerOne.h"

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

const int kSendDataPins[] = {7, 8, /*9,*/ 10};

const int kSendIntPin = 6;
const int kBuzzPin = 3;
const int kMpuI2cIntPin = 2;
const int kMuxSelectPins[] = {4, 5};
const int kLedPins[] = {A2, A3, A1};

#define kStaticIp "192.168.1.50" 
#define kNetSSID "VPS" 
#define kNetPass "milorad__senic"
#define kDestIp "192.168.1.3" 
#define kSrcDestPort "12345"

////////////////////////////////////////////////////////////////////

class ParallelReal {
 public:
  static void Print(const char* msg) {
    while (*msg) {
      SendByte(*msg);
      msg++;
    }
  }

  static void Print(int x) {
    Print(String(x).c_str());
  }

  static void Print(float x) {
    Print((int)x);
    x -= (int)x;
    x *= 100;
    Print(".");
    Print((int)x);
  }
  
  static void Println(const char* msg) {
    Print(msg);
    SendByte(13);
    SendByte(10);
  }

  static void Println(int x) {
    Println(String(x).c_str());
  }

  static void Println(float x) {
    Print((int)x);
    x -= (int)x;
    x *= 100;
    Print(".");
    Println((int)x);
  }
 
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
      digitalWrite(kSendDataPins[i], (xl & mask) ? HIGH : LOW);
      mask <<= 1;
    }
    delay(1);
    digitalWrite(kSendIntPin, HIGH);
    delay(1);
    digitalWrite(kSendIntPin, LOW);
    delay(1);
  }
};

class Parallel {
 public:
  static void Print(const char* msg) {
    Serial.print(msg);
  }

  static void Print(int x) {
    Serial.print(x);
  }

  static void Print(float x) {
    Serial.print(x);
  }
  
  static void Println(const char* msg) {
   Serial.println(msg);
  }

  static void Println(int x) {
    Serial.println(x);
  }

  static void Println(float x) {
    Serial.println(x);
  }
 
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
      digitalWrite(kSendDataPins[i], (xl & mask) ? HIGH : LOW);
      mask <<= 1;
    }
    delay(1);
    digitalWrite(kSendIntPin, HIGH);
    delay(1);
    digitalWrite(kSendIntPin, LOW);
    delay(1);
  }
};

////////////////////////////////////////////////////////////////////

MPU6050 mpu[3];
volatile int mpuPacketSize[3];
volatile Quaternion q;  
volatile VectorFloat gravity[3];
volatile uint8_t fifoBuffer[3][64];

inline void SwitchToMpu(int idx) {
  digitalWrite(kMuxSelectPins[0], idx & 1 ? HIGH : LOW);
  digitalWrite(kMuxSelectPins[1], idx & 2 ? HIGH : LOW);
  delay(500);
}

volatile boolean dmpInterruptHappened = false;

void DmpInterruptTriggered() {
  dmpInterruptHappened = true;
}

uint32_t ni = 0;
uint32_t no[3] = {0,0,0};

void DmpInterruptHandler() {
  ni++;
  //Parallel::Println("DMPInterrupt()");
  bool changed = false;
  for (int i = 0; i < 3; i++) {
    SwitchToMpu(i);
//    Parallel::Println(i);
    int int_status = mpu[i].getIntStatus();
//    Parallel::Print("GOT STATUS: "); Parallel::Println(int_status);
    int fifo_count = mpu[i].getFIFOCount();
//    Parallel::Println(fifo_count);
    // overflow?
    if ((fifo_count & 0x10) || fifo_count == 1024) {
//      Parallel::Println("overflow");
      mpu[i].resetFIFO();
      no[i]++;
    } else if (int_status & 0x02) {
//      Parallel::Println("ok");
      if (fifo_count >= mpuPacketSize[i]) {
//        Parallel::Println("full");
        changed = true;
        mpu[i].getFIFOBytes(fifoBuffer[i], mpuPacketSize[i]);
        fifo_count -= mpuPacketSize[i];
        mpu[i].dmpGetQuaternion(&q, fifoBuffer[i]);
        mpu[i].dmpGetGravity(&gravity[i], &q);
      }
    } else {
//      Parallel::Println("nista");
    }
  }
//  
//  if (changed) {
//    for (int i = 0; i < 3; i++) {
//      Parallel::Print(gravity[i].x);
//      Parallel::Print("\t");
//      Parallel::Print(gravity[i].y);
//      Parallel::Print("\t");
//      Parallel::Print(gravity[i].z);
//      Parallel::Print("\t");
//    
//    }
//    Parallel::Println("");
//  }
}

void InitMpus() {
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

  Parallel::Println("InitMpus");
  
  boolean status = true;
  for(int i = 0; i < 3; i++) {
    Parallel::Print("----- MPU: "); Parallel::Print(i); Parallel::Println(" -----");
    SwitchToMpu(i);
    Parallel::Println("Init...");
    mpu[i].initialize();
    Parallel::Println("Test...");
    if (mpu[i].testConnection()) {
      analogWrite(kLedPins[i], 128);
      Parallel::Println("Test OK");
    } else {
      status = false;
      Parallel::Println("Test FAILED");
      break;;  
    }
    
    delay(1000);

    Parallel::Println("DMP INIT");
    int devStatus = mpu[i].dmpInitialize();

    Parallel::Print("Dev status: "); Parallel::Println(devStatus);
    if (devStatus == 0) {
      analogWrite(kLedPins[i], 255);  
      delay(1000);
      
      mpu[i].setXGyroOffset(220);
      mpu[i].setYGyroOffset(76);
      mpu[i].setZGyroOffset(-85);
      mpu[i].setZAccelOffset(1788); // 1688 factory default for my test chip

      mpu[i].setDMPEnabled(true);

      mpuPacketSize[i] = mpu[i].dmpGetFIFOPacketSize();

      mpu[i].setDLPFMode(6);
//      mpu[i].setIntEnabled(1);
//      mpu[i].setIntDataReadyEnabled(1);
//      mpu[i].setInterruptLatchClear(1);

    } else {
      analogWrite(kLedPins[i], 0);  
      status = false;
      delay(1000);
    }
  }

  if (status) {
    pinMode(kMpuI2cIntPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(kMpuI2cIntPin), DmpInterruptTriggered, RISING);
  }

  delay(1000);
}

////////////////////////////////////////////////////////////////////

const int kNumCommands = 3;

const int kCommandLeft = 0;
const int kCommandRight = 1;
const int kCommandMove = 2;
const int kCommandNone = 3;

const char* CommandMsgs[kNumCommands] = {
  "+IPD,5:LEFT",
  "+IPD,6:RIGHT",
  "+IPD,5:MOVE"
};

class WiFi {
 public:  
  static Init() {
    delay(1000);
    FlushRx();
    // Reset.
    Serial.println("AT+RST");
    tone(kBuzzPin, 1568, 300);
    delay(5000);
    FlushRx();
    // Disable echoing.
    Serial.println("ATE0");
    delay(1000);
    FlushRx();
    // Disable sleep mode.
    Serial.println("AT+SLEEP=0");
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
    // Set station mode.
    Serial.println("AT+CWMODE_CUR=1");
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
    // Disable auto-connect.
    Serial.println("AT+CWAUTOCONN=0");
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
    // Disable DHCP without saving changes in the flash.
    Serial.println("AT+CWDHCP_CUR=1,0");
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
    // Set static IP without saving changes in the flash.
    Serial.println("AT+CIPSTA=\"" kStaticIp "\"");
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
    // Connecto to an AP without saving changes in the flash.
    Serial.println("AT+CWJAP_CUR=\"" kNetSSID "\",\"" kNetPass "\"");
    tone(kBuzzPin, 1568, 300);
    delay(15000);
    FlushRx();
    // Disable multiple connections.
    Serial.println("AT+CIPMUX=0");
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
    // Set normal transmission mode.
    Serial.println("AT+CIPMODE=0");
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
    // Establish UDP transmission.
    Serial.println("AT+CIPSTART=\"UDP\",\"" kDestIp "\"," kSrcDestPort "," kSrcDestPort);
    tone(kBuzzPin, 1568, 300);
    delay(1000);
    FlushRx();
  }
  
  static void Send(const char* msg) {
    int n = strlen(msg);
    Serial.print("AT+CIPSEND=");
    Serial.println(String(n).c_str());
    delay(500);
    Serial.print(msg);
    Serial.println();
    delay(500);
  }

  // Every second line will probably be empty, because ESP
  // probably always returns both /13 and /10, but we don't
  // really care.
  static void ReadLineToBuffer() {
    Parallel::SendByte('<');
    int pos = 0;
    while (true) {
      if (!Serial.available()) continue;
      char c = Serial.read();
      if (c == '\n' || c =='\r') {
        break;
      }
      buffer[pos++] = c;
      Parallel::SendByte(c);
    }
    buffer[pos] = 0;
    Parallel::SendByte('>');
    Parallel::SendByte(13);
    Parallel::SendByte(10);
  } 

  static int ParseCommand() {
    for (int i = 0; i < kNumCommands; i++) {
      if (strcmp(CommandMsgs[i], buffer) == 0) {
        return i;
      }
    }
    return kCommandNone;
  }

 private:  
  static char buffer[1024];
  
  static void FlushRx() {
    while (Serial.available()) {
      char c = Serial.read();
      Parallel::SendByte(c);  
    }  
  }
};

char WiFi::buffer[1024];

////////////////////////////////////////////////////////////////////

//class MPU {
// public:
//  static void SwitchToMPU(int idx) {
//    digitalWrite(kMuxSelectPins[0], idx & 1 ? HIGH : LOW);
//    digitalWrite(kMuxSelectPins[1], idx & 2 ? HIGH : LOW);
//    delay(500);
//  }
//
//  static void IntRoutine() {
//    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
//      Wire.begin();
//    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
//      Fastwire::setup(400, true);
//    #endif
//    
//    Parallel::Println("Interrupt!");
//    int16_t ax, ay, az;
//    int16_t gx, gy, gz;
//    for (int i = 0; i < 3; i++) {
//      SwitchToMPU(i);
//      mpu_[i].getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
//      Parallel::Print(ax); Parallel::Print("\t");
//      Parallel::Print(ay); Parallel::Print("\t");
//      Parallel::Print(az); Parallel::Print("\t");
//      Parallel::Print(gx); Parallel::Print("\t");
//      Parallel::Print(gy); Parallel::Print("\t");
//      Parallel::Print(gz); Parallel::Print("\t");
//    }
//    Parallel::Println("");
//  }
//  
//  static void Init() {
//    bool ok = true;
//    
//    for (int i = 0; i < 3; i++) {
//      SwitchToMPU(i);
//      mpu_[i].initialize();
//      delay(1);
//      if (ok = mpu_[i].testConnection()) {
//        Parallel::Println("CONNECTED");
//      } else {
//        Parallel::Println("CONNECTION FAILED");
//        break;
//      }
//      delay(1);
////      mpu_[i].setDLPFMode(1);
////      delay(1);
////      mpu_[i].setIntEnabled(1);
////      delay(1);
////      mpu_[i].setIntDataReadyEnabled(1);
////      delay(1);
////      mpu_[i].setInterruptLatchClear(1);
////      delay(1);
//    }
////    
////    if (ok) {
////      pinMode(kMpuI2cIntPin, INPUT);
////      attachInterrupt(digitalPinToInterrupt(kMpuI2cIntPin), IntRoutine, RISING);
////    }
//  }
//  
//  static void Fetch() {}
//  
//  static void Filter() {} 
//  
//  static void GetZAngles(double& a, double& b, double& c) {
//    a = random(9000) * 0.01;
//    b = random(9000) * 0.01;
//    c = random(9000) * 0.01;
//  }
// private:
//  static MPU6050 mpu_[3];
//};
//
//MPU6050 MPU::mpu_[3];
//
//////////////////////////////////////////////////////////////////////
//
//void TimerCallback() {
//  MPU::Fetch();
//  MPU::Filter();
//  double az, bz, cz;
//  MPU::GetZAngles(az, bz, cz);
//  // Calculate motor speeds.
//  // Use Parallel to send the speeds to Arduino 2.
//  WiFi::Send((String(az, 2) + " : " + String(bz, 2) + " : " + String(cz, 2)).c_str()); 
//}

////////////////////////////////////////////////////////////////////

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(kSendDataPins[i], OUTPUT);
    digitalWrite(kSendDataPins[i], LOW);
  }
  pinMode(kSendIntPin, OUTPUT);
  digitalWrite(kSendIntPin, LOW);
  pinMode(kBuzzPin, OUTPUT);
  digitalWrite(kBuzzPin, LOW);

  for (int i = 0; i < 2; i++) {
    pinMode(kMuxSelectPins[i], OUTPUT);  
  }

  for (int i = 0; i < 3; i++) {
    pinMode(kLedPins[i], OUTPUT);
    analogWrite(kLedPins[i], 0);
  }
  
  Serial.begin(115200);
  
  //delay(10000);

  InitMpus();
  
//  WiFi::Init();

//  delay(5000);
//
//  randomSeed(analogRead(0));
//  
//  Timer1.initialize(1000000);
//  Timer1.pwm(9, 512);
//  Timer1.attachInterrupt(MPU::IntRoutine);

//  MPU::Init();
  
}

//uint8_t i = 0;
void loop() {
  if (dmpInterruptHappened) {
    dmpInterruptHappened = false; 
    DmpInterruptHandler();
  }

  if (ni == 100) {
    Serial.println(no[0]);
    Serial.println(no[1]);
    Serial.println(no[2]);
  }
//  analogWrite(kLedPins[0], 255);
//  delay(500);
//  MPU::IntRoutine();
//  analogWrite(kLedPins[0], 0);
//  delay(500);
  
//  if (Serial.available()) {
//    Parallel::SendByte(Serial.read());  
//  }

//  Parallel::SendByte(i++);
//  delay(300);

//  WiFi::ReadLineToBuffer();
//  switch (WiFi::ParseCommand()) {
//    case kCommandLeft: {
//      Parallel::SendByte('L');
//      Parallel::SendByte(13);
//      Parallel::SendByte(10);
//      WiFi::Send("LEFT");
//      break;
//    }
//    case kCommandRight: {
//      Parallel::SendByte('R');
//      Parallel::SendByte(13);
//      Parallel::SendByte(10);
//      WiFi::Send("RIGHT");
//      break;
//    }
//    case kCommandMove: {
//      Parallel::SendByte('M');
//      Parallel::SendByte(13);
//      Parallel::SendByte(10);
//      WiFi::Send("MOVE");
//      break;
//    }
//    default: {
//      Parallel::SendByte('N');
//      Parallel::SendByte(13);
//      Parallel::SendByte(10);
//      break;
//    }
//  }
}
