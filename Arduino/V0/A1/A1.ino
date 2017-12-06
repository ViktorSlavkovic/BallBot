const int kWiFiResetPin = 11;
const int kSendDataPins[] = {7, 8, 9, 10};
const int kSendIntPin = 6;
const int kBuzzPin = 3;
const int kMpuI2cIntPin = 2;
const int kMuxSelectPins[] = {4, 5};

const int kLedPins[] = {A2, A3, A1};

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(kSendDataPins[i], OUTPUT);
    digitalWrite(kSendDataPins[i], LOW);
  }
  pinMode(kSendIntPin, OUTPUT);
  digitalWrite(kSendIntPin, LOW);
  pinMode(kBuzzPin, OUTPUT);
  digitalWrite(kBuzzPin, LOW);
  
  pinMode(kWiFiResetPin, OUTPUT);
  digitalWrite(kWiFiResetPin, HIGH);

  for (int i = 0; i < 3; i++) {
    pinMode(kLedPins[i], OUTPUT);
    analogWrite(kLedPins[i], 255);
  }
  
  delay (10000);

  Serial.begin(115200);
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

uint8_t i = 0;

bool did1 = false;
bool did2 = false;

void loop() {

  if (millis() > 20000 && !did1) {
    did1 = true;
    digitalWrite(kWiFiResetPin, LOW);
    delay(10);
    digitalWrite(kWiFiResetPin, HIGH);
    delay(10);
  }
  
  if (millis() > 30000 && !did2) {
    Serial.write('A');
    Serial.write('T');
    Serial.write(13);
    Serial.write(10); 
    did2 = true; 
  } 
//    SendByte(i++);
//    delay(300);
  if (Serial.available()) {
    SendByte(Serial.read());
    delay(10);
  }
}
