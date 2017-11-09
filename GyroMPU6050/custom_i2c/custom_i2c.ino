//#include <cinttypes>

const int kSdaPin = 3; //D6;
const int kSdaReadPin = 4; //D7;
const int kSclPin = 2; //D5;

inline void Init() {
  digitalWrite(kSdaPin, HIGH);
  digitalWrite(kSclPin, HIGH);
  //delay(1);
}

inline bool WriteByte(uint8_t reg, uint8_t val) {
  // Start ///////////////////
  // SDA --\__ ... ___
  // SCL ----- ... \__
  digitalWrite(kSdaPin, LOW);
  //delay(1);
  digitalWrite(kSclPin, LOW);
  //delay(1);
  
  // Send Device Address and W bit (0): 0x68(7) 0(1) -flip-> 0x0B
  int address = 0x0B;
  for (int i = 0; i < 8; i++) {
    digitalWrite(kSdaPin, address & 1 ? HIGH : LOW);
    digitalWrite(kSclPin, HIGH);
    //delay(1);
    digitalWrite(kSclPin, LOW);
    digitalWrite(kSdaPin, LOW);
    address >>= 1;
    //delay(1);
  }
  
  // Get ACK.
  //pinMode(kSdaPin, INPUT);
  digitalWrite(kSdaPin, HIGH); // Release
  
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  
  int nack = digitalRead(kSdaReadPin);
  if (nack) {
    return false;
  } 
  digitalWrite(kSclPin, LOW);
  //pinMode(kSdaPin, OUTPUT);
  //delay(1);
  
  // Send register address.
  uint8_t flipped_reg = 0;
  for (int i = 0; i < 8; i++) {
    flipped_reg |= reg & 1;
    flipped_reg <<= 1;
    reg >>= 1;
  }
  for (int i = 0; i < 8; i++) {
    digitalWrite(kSdaPin, flipped_reg & 1 ? HIGH : LOW);
    digitalWrite(kSclPin, HIGH);
    //delay(1);
    digitalWrite(kSclPin, LOW);
    digitalWrite(kSdaPin, LOW);
    flipped_reg >>= 1;
    //delay(1);
  }
  
  // Get ACK.
  //pinMode(kSdaPin, INPUT);
  digitalWrite(kSdaPin, HIGH); // Release
  
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  nack = digitalRead(kSdaReadPin);
  if (nack) {
    return false;
  } 
  digitalWrite(kSclPin, LOW);
  //pinMode(kSdaPin, OUTPUT);
  //delay(1);
  
  // Send byte value.
  uint8_t flipped_val = 0;
  for (int i = 0; i < 8; i++) {
    flipped_val |= val & 1;
    flipped_val <<= 1;
    val >>= 1;
  }
  for (int i = 0; i < 8; i++) {
    digitalWrite(kSdaPin, flipped_val & 1 ? HIGH : LOW);
    digitalWrite(kSclPin, HIGH);
    //delay(1);
    digitalWrite(kSclPin, LOW);
    digitalWrite(kSdaPin, LOW);
    flipped_val >>= 1;
    //delay(1);
  }
  
  // Get ACK.
  //pinMode(kSdaPin, INPUT);
  digitalWrite(kSdaPin, HIGH); // Release
  
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  nack = digitalRead(kSdaReadPin);
  if (nack) {
    return false;
  } 
  digitalWrite(kSclPin, LOW);
  //pinMode(kSdaPin, OUTPUT);
  //delay(1);
  
  digitalWrite(kSdaPin, LOW); // Retake
  //delay(1);
    
  // Stop.
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  digitalWrite(kSdaPin, HIGH);
  //delay(1);

  return true;
}

inline bool ReadByte(uint8_t reg, uint8_t* val) {
  // Start ///////////////////
  // SDA --\__ ... ___
  // SCL ----- ... \__
  digitalWrite(kSdaPin, LOW);
  //delay(1);
  digitalWrite(kSclPin, LOW);
  //delay(1);
  
  // Send Device Address and W bit (0): 0x68(7) 0(1) -flip-> 0x0B
  int address = 0x0B;
  for (int i = 0; i < 8; i++) {
    digitalWrite(kSdaPin, address & 1 ? HIGH : LOW);
    digitalWrite(kSclPin, HIGH);
    //delay(1);
    digitalWrite(kSclPin, LOW);
    digitalWrite(kSdaPin, LOW);
    address >>= 1;
    //delay(1);
  }
  
  // Get ACK.
  //pinMode(kSdaPin, INPUT);
  digitalWrite(kSdaPin, HIGH); // Release
  
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  int nack = digitalRead(kSdaReadPin);
  while (nack) {
    nack = digitalRead(kSdaReadPin);
  }
  Serial.println("Passed!");
  if (nack) {
    Serial.println("HERE 1");
    while(1) {}
    return false;
  } 
  digitalWrite(kSclPin, LOW);
  //pinMode(kSdaPin, OUTPUT);
  //delay(1);
  
  // Send register address.
  uint8_t flipped_reg = 0;
  for (int i = 0; i < 8; i++) {
    flipped_reg |= reg & 1;
    flipped_reg <<= 1;
    reg >>= 1;
  }
  for (int i = 0; i < 8; i++) {
    digitalWrite(kSdaPin, flipped_reg & 1 ? HIGH : LOW);
    digitalWrite(kSclPin, HIGH);
    //delay(1);
    digitalWrite(kSclPin, LOW);
    digitalWrite(kSdaPin, LOW);
    flipped_reg >>= 1;
    //delay(1);
  }
  
  // Get ACK.
  //pinMode(kSdaPin, INPUT);
  digitalWrite(kSdaPin, HIGH); // Release
  
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  nack = digitalRead(kSdaReadPin);
  if (nack) {
    Serial.println("HERE 2");
    return false;
  } 
  digitalWrite(kSclPin, LOW);
  //pinMode(kSdaPin, OUTPUT);
  //delay(1);
  
  // Start again///////////////////
  // SDA --\__ ... ___
  // SCL ----- ... \__
  digitalWrite(kSdaPin, HIGH);
  digitalWrite(kSclPin, HIGH);
  digitalWrite(kSdaPin, LOW);
  digitalWrite(kSdaPin, LOW);
  //delay(1);
  digitalWrite(kSclPin, LOW);
  //delay(1);
  
  // Send Device Address and R bit (1): 0x68(7) 1(1) -flip-> 0x8B
  address = 0x8B;
  for (int i = 0; i < 8; i++) {
    digitalWrite(kSdaPin, address & 1 ? HIGH : LOW);
    digitalWrite(kSclPin, HIGH);
    //delay(1);
    digitalWrite(kSclPin, LOW);
    digitalWrite(kSdaPin, LOW);
    address >>= 1;
    //delay(1);
  }
  
  // Get ACK and recv byte value.
  //pinMode(kSdaPin, INPUT);
  digitalWrite(kSdaPin, HIGH); // Release
  
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  nack = digitalRead(kSdaReadPin);
  if (nack) {
    Serial.println("HERE 3");
    return false;
  } 
  digitalWrite(kSclPin, LOW);
  //pinMode(kSdaPin, OUTPUT);
  //delay(1);
  
  uint8_t byte_val = 0;
  for (int i = 0; i < 8; i++) {
    digitalWrite(kSclPin, HIGH);
    //delay(1);
    int x = digitalRead(kSdaReadPin);
    digitalWrite(kSclPin, LOW);
    if (x) {
      byte_val++;
    }
    byte_val <<= 1;
    //delay(1);
  }
  //pinMode(kSdaPin, OUTPUT);

  // Send NACK.
  digitalWrite(kSdaPin, HIGH);
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  digitalWrite(kSclPin, LOW);
  digitalWrite(kSdaPin, LOW);
  //delay(1);
  
  // Stop.
  digitalWrite(kSclPin, HIGH);
  //delay(1);
  digitalWrite(kSdaPin, HIGH);
  //delay(1);

  if (val) {
    *val = byte_val;
  }
  return true;
}

bool success = true;

void setup() {
  pinMode(kSdaReadPin, INPUT);
  pinMode(kSdaPin, OUTPUT);
  pinMode(kSclPin, OUTPUT);

  Serial.begin(38400);

  delay(3000);
  Serial.println();
  Serial.println("<><><><><><> START <><><><><><>");
  
  Init();

  uint8_t val, val1;

  // Set PWR_MGMT_1
  success = success && ReadByte(0x6B /* PWR_MGMT_1 */, &val);

  if (success) {
    Serial.println("Read PWR_MGMT_1 OK!");
    Serial.println(val);
  } else {
    Serial.println("Read PWR_MGMT_1 FAILED!");
    return;
  }
  
  // CLK_SEL = 1 (PLL with X axis gyroscope reference)
  val &= ~0 << 3;
  val += 1;
  // SLEEP = 0
  val &= ~0x40;
  success = success && WriteByte(0x6B /* PWR_MGMT_1 */, val); 
  
  if (success) {
    Serial.println("Set PWR_MGMT_1 OK!");
    Serial.println(val);
  } else {
    Serial.println("Set PWR_MGMT_1 FAILED!");
    return;
  }
  
  success = success && ReadByte(0x6B /* PWR_MGMT_1 */, &val1);
  
  if (success) {
    Serial.println("Read PWR_MGMT_1 again OK!");
    Serial.println(val1);
  } else {
    Serial.println("Read PWR_MGMT_1 FAILED!");
    return;
  }
  
  success = success && (val == val1);
  
  if (!success) { 
    Serial.print("Test FAILED!");
    return;
  }
  
  // Set GYRO_CONFIG
  success = success && ReadByte(0x1B /* GYRO_CONFIG */, &val);
  // FS_SEL = 0 (+- 250 deg/s)
  val &= ~0x18;
  success = success && WriteByte(0x1B /* GYRO_CONFIG */, val); 

  if (success) {
    Serial.println("Set GYRO_CONFIG OK!");
  } else {
    Serial.println("Set GYRO_CONFIG FAILED!");
    return;
  }
  
  // Set ACCEL_CONFIG
  success = success && ReadByte(0x1B /* ACCEL_CONFIG */, &val);
  // AFS_SEL = 0 (+- 2g)
  val &= ~0x18;
  success = success && WriteByte(0x1B /* ACCEL_CONFIG */, val); 

  if (success) {
    Serial.println("Set ACCEL_CONFIG OK!");
  } else {
    Serial.println("Set ACCEL_CONFIG FAILED!");
    return;
  }
  
  // Test connection
  success = success && ReadByte(0x75 /* WHO_AM_I */, &val);
  val >>=1;
  val &= ~0xC0;
  success = success && (val == 0x34);

  if (success) {
    Serial.println("TEST OK!");
  } else {
    Serial.println("TEST FAILED!");
    return;
  }
}

uint8_t gxl, gxh, gyl, gyh, gzl, gzh;
uint8_t axl, axh, ayl, ayh, azl, azh;
uint16_t ax, ay, az, gx, gy, gz;

const double kGyroScale = 131.068; // (2^15 - 1) / 250 deg/sec
const double kAccelScale =  16383.5; // (2^15 - 1) / 2 g/sec

void loop() {
  if (!success) return;
  ReadByte(0x44, &gxl);
  ReadByte(0x43, &gxh);
  ReadByte(0x46, &gyl);
  ReadByte(0x45, &gyh);
  ReadByte(0x48, &gzl);
  ReadByte(0x47, &gzh); 

  ReadByte(0x3C, &axl);
  ReadByte(0x3B, &axh);
  ReadByte(0x3E, &ayl);
  ReadByte(0x3D, &ayh);
  ReadByte(0x40, &azl);
  ReadByte(0x3F, &azh);

  ax = ((uint16_t)axh << 8) | axl;
  ay = ((uint16_t)ayh << 8) | ayl;
  az = ((uint16_t)azh << 8) | azl;
  gx = ((uint16_t)gxh << 8) | gxl;
  gy = ((uint16_t)gyh << 8) | gyl;
  gz = ((uint16_t)gzh << 8) | gzl;

  Serial.print("a/g:\t");
  
  Serial.print(ax / kAccelScale); Serial.print("\t");
  Serial.print(ay / kAccelScale); Serial.print("\t");
  Serial.print(az / kAccelScale); Serial.print("\t");
  
  Serial.print(gx / kGyroScale); Serial.print("\t");
  Serial.print(gy / kGyroScale); Serial.print("\t");
  Serial.println(gz / kGyroScale);
}

