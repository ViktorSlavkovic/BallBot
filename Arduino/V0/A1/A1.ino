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

}

void loop() {
  // put your main code here, to run repeatedly:

}
