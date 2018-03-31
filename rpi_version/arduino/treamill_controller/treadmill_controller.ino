const int kIntPin = 2;
const int kData0Pin = 4;
const int kData1Pin = 5;
const int kData2Pin = 6;

const int kModePin0 = 11;
const int kModePin1 = 10;
const int kModePin2 = 12;

const int kStepPin = 9;
const int kDirPin = 8;
const int kEnablePin = 13;
const int kSlowCnt = 100;
const long kDtMsMax = 50000;
const long kDtMsMin = 4000;
const long kDtMsStep = 1000;

bool dir = false;
bool shouldLoop = false;
long dt_ms = kDtMsMax;
int cnt = kSlowCnt;

void SetMode(int mode) {
  digitalWrite(kModePin0, (mode & 1) ? HIGH : LOW);
  digitalWrite(kModePin1, (mode & 2) ? HIGH : LOW);
  digitalWrite(kModePin2, (mode & 4) ? HIGH : LOW);
}

void Interrupt() {
  int p0 = digitalRead(kData0Pin);
  int p1 = digitalRead(kData1Pin);
  int p2 = digitalRead(kData2Pin);
  
  int tmp = p0 + p1 + p2;

  Serial.println(String(p0) + " " + String(p1) + " " + String(p2) + " : " + String(tmp));
  
  if (tmp >= 2) {
    dir = false;
    cnt = kSlowCnt;
    dt_ms = kDtMsMax;
    shouldLoop = true;
  } else {
    shouldLoop = false;
  }
}

void setup() {
  
  pinMode(kStepPin, OUTPUT);
  pinMode(kDirPin, OUTPUT);
  pinMode(kEnablePin, OUTPUT);

  pinMode(kData0Pin, INPUT);
  pinMode(kData1Pin, INPUT);
  pinMode(kData2Pin, INPUT);
  pinMode(kIntPin, INPUT);

  pinMode(kModePin0, INPUT);
  pinMode(kModePin1, INPUT);
  pinMode(kModePin2, INPUT);

  SetMode(2); // 1/4 step
    
  //attachInterrupt(kIntPin, Interrupt, RISING);
  
  digitalWrite(kStepPin, dir ? HIGH : LOW);
  digitalWrite(kDirPin, LOW);
  digitalWrite(kEnablePin, LOW);

  Serial.begin(115200);
  Serial.println("TEST");
}

int rr = 0;

void loop() {

  int r = digitalRead(kIntPin);
  if (rr == 0 && r == 1) {
    Interrupt();
  }
  
  if (!shouldLoop) {
    return;  
  }
  
  digitalWrite(kDirPin, dir ? HIGH : LOW);
  digitalWrite(kStepPin, HIGH);

  delayMicroseconds(500);

  digitalWrite(kStepPin, LOW);

  delay(dt_ms / 1000);
  delayMicroseconds(dt_ms % 1000);

  cnt--;
  if (cnt == 0) {
    dt_ms -= kDtMsStep;
    if (dt_ms < kDtMsMin) {
      dt_ms = kDtMsMax;
      dir = !dir;
    }
    cnt = kSlowCnt;
  }
}
