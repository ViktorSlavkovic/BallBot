const int kSendDataPins[] = {3, 4, 5, 6};
const int kSendIntPin = 2;
const int kBuzzPin = 7;

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(kSendDataPins[i], OUTPUT);
    digitalWrite(kSendDataPins[i], LOW);
  }
  pinMode(kSendIntPin, OUTPUT);
  digitalWrite(kSendIntPin, LOW);
  pinMode(kBuzzPin, OUTPUT);
  digitalWrite(kBuzzPin, LOW);

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

void loop() {
//    SendByte(i++);
//    delay(300);
  if (Serial.available()) {
    SendByte(Serial.read());
    delay(10);
  }
}
