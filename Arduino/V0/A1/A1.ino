#include <I2Cdev.h>
#include <Plotter.h>

// Include this if DMP is being used.
#include "MPU6050_6Axis_MotionApps20.h"
// Otherwise include this one.
//#include <MPU6050.h>


const double kGyroScale = 131.068; // (2^15 - 1) / 250 deg/sec
const double kAccelScale =  16383.5; // (2^15 - 1) / 2 g/sec
const double kPi = 3.1415926535897932384626;

// Does not exist any more.
const int kWiFiResetPin = 11;

const int kSendDataPins[] = {7, 8, 9, 10};
const int kSendIntPin = 6;
const int kBuzzPin = 3;
const int kMpuI2cIntPin = 2;
const int kMuxSelectPins[] = {9, 10};

const int kLedPins[] = {A2, A3, A1};

MPU6050 mpus[3] = {0x68, 0x68, 0x68};

int startInterruptCounter = 0;

int16_t ax[3], ay[3], az[3];
int16_t gx[3], gy[3], gz[3];

void mpuInterruptRoutine() {
  Serial.println("Interrupt!!!!!!");
  if (startInterruptCounter < 3) {
    analogWrite(kLedPins[startInterruptCounter++], 255);  
  }
}

void setup() {
  
  // Initial values on a "bus" between A1 and A2.
  for (int i = 0; i < 3; i++) {
    pinMode(kSendDataPins[i], OUTPUT);
    digitalWrite(kSendDataPins[i], LOW);
  }
  // Intial value of an interupt pin is LOW, so interrupt is on RISING edge.
  pinMode(kSendIntPin, OUTPUT);
  digitalWrite(kSendIntPin, LOW);

  // Intialize the interrupt pin.
  pinMode(kMpuI2cIntPin, INPUT);
  digitalWrite(kMpuI2cIntPin, LOW);


  // Buzzer setup.
  pinMode(kBuzzPin, OUTPUT);
  digitalWrite(kBuzzPin, LOW);

  // Reset the wifi connection.
//  pinMode(kWiFiResetPin, OUTPUT);
//  digitalWrite(kWiFiResetPin, HIGH);

  for (int i = 0; i < 3; i++) {
    pinMode(kLedPins[i], OUTPUT);
    analogWrite(kLedPins[i], 0);
  }

  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif


  Serial.begin(115200);
  
  pinMode(kMuxSelectPins[0], OUTPUT);
  pinMode(kMuxSelectPins[1], OUTPUT);

  // Intialization of MPUs,
  for (int i = 0; i < 3; i++) {
    digitalWrite(kMuxSelectPins[0], boolean(i & 0x1));
    digitalWrite(kMuxSelectPins[1], boolean(i & 0x2));


    Serial.print(F("Initializing MPU "));
    Serial.println(i);
    mpus[i].initialize();    
    Serial.println(mpus[i].testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

//    Serial.print(F("Initializing DMP on MPU "));
//    Serial.println(i);
//    int dmpInitStatus = mpus[1].dmpInitialize();
//
//    if (dmpInitStatus == 0) {
//      // turn on the DMP, now that it's ready
//      Serial.println(F("Enabling DMP..."));
//      mpus[i].setDMPEnabled(true);
//
//      attachInterrupt(0, mpuInterruptRoutine, RISING);
//      
//    } else {
//      Serial.print(F("DMP Initialization failed (code "));
//      Serial.print(dmpInitStatus);
//      Serial.println(F(")"));  
//    }
  }
  
  delay (1000);  
}

void SendLower3b(uint8_t xl) {
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

void SendByte(uint8_t x) {
  SendLower3b(x);
  x >>= 3;
  SendLower3b(x);
  x >>= 3;
  SendLower3b(x);
}

//uint8_t i = 0;
//
//bool did1 = false;
//bool did2 = false;


void loop() {
  Serial.println("*********************************");
  for (int i = 0; i < 3; i++) {
    digitalWrite(kMuxSelectPins[0], boolean(i & 0x1));
    digitalWrite(kMuxSelectPins[1], boolean(i & 0x2));

    mpus[i].getMotion6(ax+i, ay+i, az+i, gx+i,gy+i, gz+i);
    
    Serial.println(i);
    Serial.print(ax[i] / kAccelScale); Serial.print("\t");
    Serial.print(ay[i] / kAccelScale); Serial.print("\t");
    Serial.print(az[i] / kAccelScale); Serial.println("\t");

    double dax = ax[i] / kAccelScale;
    double day = ay[i] / kAccelScale;
    double daz = az[i] / kAccelScale;

    double daxy = sqrt(dax*dax + day*day);
    double angleZ = atan(daxy/daz);
    angleZ = abs(angleZ);
    angleZ *= (180.0 /kPi);  


    double daxz = sqrt(dax*dax + daz*daz);
    double angleY = atan(daxz/day);
    angleY = abs(angleY);
    angleY *= (180.0 /kPi);  

    double dayz = sqrt(day*day + daz*daz);
    double angleX = atan(dayz/dax);
    angleX = abs(angleX);
    angleX *= (180.0 /kPi);  


    Serial.print(gx[i] / kGyroScale + 5.16); Serial.print("\t");
    Serial.print(gy[i] / kGyroScale + 1.65); Serial.print("\t");
    Serial.print(gz[i] / kGyroScale + 5.79); Serial.println("\t");
    Serial.print("Angle X: ");Serial.print(angleX);Serial.print("\t");
    Serial.print("Angle Y: ");Serial.print(angleY);Serial.print("\t");
    Serial.print("Angle Z: ");Serial.print(angleZ);Serial.println("\t");
  }
}

//void loop() {
//
////  if (millis() > 20000 && !did1) {
////    did1 = true;
////    digitalWrite(kWiFiResetPin, LOW);
////    delay(10);
////    digitalWrite(kWiFiResetPin, HIGH);
////    delay(10);
////  }
////  
////  if (millis() > 30000 && !did2) {
////    Serial.write('A');
////    Serial.write('T');
////    Serial.write(13);
////    Serial.write(10); 
////    did2 = true; 
////  } 
////    SendByte(i++);
////    delay(300);
////  if (Serial.available()) {
////    SendByte(Serial.read());
////    delay(10);
////  }
//  
//
//
//}
