const int kStepPins[] = {12, 10, 8};
const int kDirPins[] = {11, 9, 7};

const int kRecvDataPins[] = {3, 4, /*5,*/ 6};
const int kRecvIntPin = 2;

volatile int recv_state = 0; // 0 - clear, 1 - 3b, 2 - 6b
volatile uint8_t recv_byte = 0x55;
volatile uint8_t recv_correct = 0;
volatile unsigned long t = millis();


void RecvIntRoutine() {
  unsigned long t1 = millis();
  unsigned long dt = t1 - t;
  if (dt < 1) return;
  if (dt > 300) recv_state = 0; 
  
//  Serial.println("RecvIntRoutine()");
  uint8_t x = 0;
  for (int i = 0; i < 3; i++) {
    uint8_t mask = digitalRead(kRecvDataPins[i]) << i;
    x |= mask;
  }

  switch (recv_state) {
    case 0: {
      recv_byte = x;
      recv_state++;
      break;
    }
    case 1: {
      recv_byte |= x << 3;
      recv_state++;
      break;
    }
    case 2: {
      recv_byte |= x << 6;
      recv_state = 0;
//      Serial.print((int)recv_correct++); Serial.print("\t"); Serial.println((int)recv_byte);
      Serial.write(recv_byte);
      break;
    }
  }

  t = t1;
}

int dir = HIGH;
const int stepTime[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

void setup() {
  Serial.begin(115200);
  
  for (int i = 0; i < 3; i++) {
    pinMode(kStepPins[i], OUTPUT);
    pinMode(kDirPins[i], OUTPUT);
    digitalWrite(kStepPins[i], dir);
    digitalWrite(kDirPins[i], dir);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(kRecvDataPins[i], INPUT);
  }
  
  pinMode(kRecvIntPin, INPUT);
  delay(3000);
  Serial.println("Attach");
  attachInterrupt(digitalPinToInterrupt(kRecvIntPin), RecvIntRoutine, RISING);
}

void loop() {
//  int t= millis();
//  int t1;
  for (int i = 0; i < 200; i++) {
//  for (int i = 8; i < 9; i++) { 
//    while ((t1 = millis()) - t < 2000) {
      for (int j = 0; j < 3; j++) {
        digitalWrite(kStepPins[j], HIGH);
      }
      delayMicroseconds(500/*stepTime[i]*/);                      
      for (int j = 0; j < 3; j++) {
        digitalWrite(kStepPins[j], LOW);
      }
      delayMicroseconds(500/*stepTime[i]*/);
//    }
//    t = t1;
//  }
  }

  dir = !dir;
  for (int i = 0; i < 3; i++) {
    digitalWrite(kDirPins[i], dir);
  }

  delay(1000);
}
//
///*  Simple Stepper Motor Control
// *      
// *  by Achim Pieters, www.studiopieters.nl
// *  
// */
//
//// defines pins numbers
//const int stepPin = 12; 
//const int dirPin = 11; 
// 
//void setup() {
//  // Sets the two pins as Outputs
//  pinMode(stepPin,OUTPUT); 
//  pinMode(dirPin,OUTPUT);
//}
//void loop() {
//  digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
//  // Makes 200 pulses for making one full cycle rotation
//  for(int x = 0; x < 200; x++) {
//    digitalWrite(stepPin,HIGH); 
//    delayMicroseconds(500); 
//    digitalWrite(stepPin,LOW); 
//    delayMicroseconds(500); 
//  }
//  delay(1000); // One second delay
//  
//  digitalWrite(dirPin,LOW); //Changes the rotations direction
//  // Makes 400 pulses for making two full cycle rotation
//  for(int x = 0; x < 400; x++) {
//    digitalWrite(stepPin,HIGH);
//    delayMicroseconds(500);
//    digitalWrite(stepPin,LOW);
//    delayMicroseconds(500);
//  }
//  delay(1000);
//}
