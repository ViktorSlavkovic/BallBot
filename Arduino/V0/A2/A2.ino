const int kStepPins[] = {12, 10, 8};
const int kDirPins[] = {11, 9, 7};

const int kRecvDataPins[] = {3, 4, /*5,*/ 6};
const int kRecvIntPin = 2;

volatile uint32_t MicroDelay[3] = {~0lu, ~0lu, ~0lu};
volatile uint8_t Dirs[3] = {0, 0, 0};

volatile int recv_state = 0; // 0 - clear, 1 - 2b, 2 - 4b, 3 - 6b
volatile uint8_t recv_byte = 0x55;
volatile uint8_t recv_correct = 0;
volatile unsigned long t = millis();

// LOG_LEVEL values:
//   0 - Serial is never used.
//   1 - Every parallel recv byte is sent over serial directly.
//   2 - Every parallel recv byte is written to serial as it's dec value and sequence number. 
#define LOG_LEVEL 1

// TODO(viktors): Implement.
void UpdateMotorDataFromRecv(uint8_t x) {
  
}

// TODO(viktors): Check timing - types and granularity.
void RecvIntRoutine() {
  unsigned long t1 = millis();
  unsigned long dt = t1 - t;
  
  #if LOG_LEVEL > 1
    Serial.println("RecvIntRoutine()");
  #endif
  
  // Is this interrupt too early?
  // TODO(viktors): Check if we really need this.
  if (dt < 1) {
    #if LOG_LEVEL > 1
      Serial.println("Here 1");
    #endif
    return;
  }
  // Is this interrupt too late - is some previous
  // interrupt missing?
  // TODO(viktors): Check the 600 value.
  if (dt > 600) {
    #if LOG_LEVEL > 1
      Serial.println("Here 2");
    #endif
    recv_state = 0; 
  }

  // Get the parallel data and build a byte.
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
    case 1:{
      recv_byte |= x << 3;
      recv_state++;
      break;
    }
    case 2: {
      recv_byte |= x << 6;
      recv_state = 0;
      #if LOG_LEVEL > 0
        Serial.write(recv_byte);
      #elif LOG_LEVEL > 1
        Serial.print((int)recv_correct++); Serial.print("\t"); Serial.println((int)recv_byte);
      #endif
      // A whole byte is recieved -> decode.
      UpdateMotorDataFromRecv(recv_byte);
      break;
    }
  }

  t = t1;
}

// DONE
inline uint32_t GearToMicors(uint8_t gear) {
  if (!gear) return ~uint64_t(0);
  return (1 + (31 - gear) * 32llu) * 100;
}

// DONE
void setup() {
  
  #if LOG_LEVEL > 0
    Serial.begin(115200);
  #endif

  for (int i = 0; i < 3; i++) {
    pinMode(kStepPins[i], OUTPUT);
    pinMode(kDirPins[i], OUTPUT);
    digitalWrite(kStepPins[i], LOW);
    digitalWrite(kDirPins[i], LOW);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(kRecvDataPins[i], INPUT);
  }
  
  pinMode(kRecvIntPin, INPUT);
  delay(3000);
  
  #if LOG_LEVEL > 0
    Serial.println("Attach");
  #endif
  
  attachInterrupt(digitalPinToInterrupt(kRecvIntPin), RecvIntRoutine, RISING);
}

bool dir = true;

// TODO(viktors): Check for possible direction change issues - maybe add a small delay.
// TODO(viktors): Implement.
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
