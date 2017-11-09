#include "I2Cdev.h"
#include "MPU6050.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

const double kGyroScale = 131.068; // (2^15 - 1) / 250 deg/sec
const double kAccelScale =  16383.5; // (2^15 - 1) / 2 g/sec
const double kPi = 3.1415926535897932384626;


int16_t ax, ay, az;
int16_t gx, gy, gz;

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
// MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high
uint8_t devAddr = 0x68;
uint8_t buffer[14];

void setup() {
  pinMode(3, OUTPUT);
  
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  ////////////////////////////////////////////////////////////////////
  // Initialize Serial Communication
  ////////////////////////////////////////////////////////////////////

  Serial.begin(38400);

  ////////////////////////////////////////////////////////////////////
  // TEMP
  ////////////////////////////////////////////////////////////////////

  // FIXME: This is a temporary test which replicates the problematic
  //        case in our I2C implementation.
//  Serial.println();
//  Serial.println("<><><><><><> START <><><><><><>");
//  delay(3000);
//  
//  uint8_t val, val1;
//  I2Cdev::readBytes(devAddr, MPU6050_RA_PWR_MGMT_1, 1, &val, 0);
//  // CLK_SEL = 1 (PLL with X axis gyroscope reference)
//  val &= ~0 << 3;
//  val += 1;
//  // SLEEP = 0
//  val &= ~0x40;
//  I2Cdev::writeBytes(devAddr, MPU6050_RA_PWR_MGMT_1, 1, &val);
//  I2Cdev::readBytes(devAddr, MPU6050_RA_PWR_MGMT_1, 1, &val1, 0);
//  
//  if (val != val1) { 
//    Serial.print("Test FAILED!");
//  } else {
//    Serial.print("Test PASSED!");
//  }
//  while (1) {}
    
  ////////////////////////////////////////////////////////////////////
  // Initialize MPU6050
  ////////////////////////////////////////////////////////////////////
  Serial.println("Initializing I2C devices...");

  // 1) Set Clock Source
  I2Cdev::writeBits(devAddr,
                    MPU6050_RA_PWR_MGMT_1 /* regAddr = 0x6B */,
                    MPU6050_PWR1_CLKSEL_BIT,
                    MPU6050_PWR1_CLKSEL_LENGTH,
                    MPU6050_CLOCK_PLL_XGYRO /* source */);
  // 2) Set Full Scale Gyro Range
  I2Cdev::writeBits(devAddr,
                    MPU6050_RA_GYRO_CONFIG /* regAddr = 0x1B */,
                    MPU6050_GCONFIG_FS_SEL_BIT,
                    MPU6050_GCONFIG_FS_SEL_LENGTH,
                    MPU6050_GYRO_FS_250 /* range */);
  // 3) Set Full Scale Accel Range
  I2Cdev::writeBits(devAddr,
                    MPU6050_RA_ACCEL_CONFIG /* regAddr = 0x1C */,
                    MPU6050_ACONFIG_AFS_SEL_BIT,
                    MPU6050_ACONFIG_AFS_SEL_LENGTH,
                    MPU6050_ACCEL_FS_2 /* range */);
  // 4) Set Sleep Enabled
  I2Cdev::writeBit(devAddr,
                   MPU6050_RA_PWR_MGMT_1 /* regAddr */,
                   MPU6050_PWR1_SLEEP_BIT,
                   false /* enabled */);

  ////////////////////////////////////////////////////////////////////
  // Verify connection
  ////////////////////////////////////////////////////////////////////

  Serial.println("Testing device connections...");

  I2Cdev::readBits(devAddr,
                   MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, buffer);

  Serial.println(buffer[0] == (devAddr >> 1)
                 ? "MPU6050 connection successful"
                 : "MPU6050 connection failed");

  ////////////////////////////////////////////////////////////////////
  // Set Accel/Gyro offset values.
  ////////////////////////////////////////////////////////////////////
  Serial.println("Updating internal sensor offsets...");
  // -76  -2359 1688  0 0 0
  //    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
  //    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
  //    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
  //    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
  //    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
  //    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
  //    Serial.print("\n");
  
  // Set gyro offset ().
//  I2Cdev::writeWord(devAddr, MPU6050_RA_XG_OFFS_USRH, 0);
//  I2Cdev::writeWord(devAddr, MPU6050_RA_YG_OFFS_USRH, 0);
//  I2Cdev::writeWord(devAddr, MPU6050_RA_ZG_OFFS_USRH, -152);
  
  //    accelgyro.setXGyroOffset(220);
  //    accelgyro.setYGyroOffset(76);
  //    accelgyro.setZGyroOffset(-85);
  //    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
  //    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
  //    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
  //    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
  //    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
  //    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
  //    Serial.print("\n");
}

double avg_gx = 0, avg_gy = 0, avg_gz = 0;
int i = 0;

void loop() {
  ////////////////////////////////////////////////////////////////////
  // Read RAW Accel/Gyro measurements from the device.
  ////////////////////////////////////////////////////////////////////

  // This is getMotion6() function unwrapped.
  // There are getAcceleration and getRotation for sepearate readings
  // as well.
  I2Cdev::readBytes(devAddr,
                    MPU6050_RA_ACCEL_XOUT_H,
                    14,
                    buffer);
  ax = (((int16_t)buffer[0]) << 8) | buffer[1];
  ay = (((int16_t)buffer[2]) << 8) | buffer[3];
  az = (((int16_t)buffer[4]) << 8) | buffer[5];
  gx = (((int16_t)buffer[8]) << 8) | buffer[9];
  gy = (((int16_t)buffer[10]) << 8) | buffer[11];
  gz = (((int16_t)buffer[12]) << 8) | buffer[13];

  ////////////////////////////////////////////////////////////////////
  // Print the values to serial port.
  ////////////////////////////////////////////////////////////////////
  
//  if (i == 10000) {
//    avg_gx /= i;
//    avg_gy /= i;
//    avg_gz /= i;
//    Serial.print(avg_gx / 131.068); Serial.print("\t");
//    Serial.print(avg_gy / 131.068); Serial.print("\t");
//    Serial.println(avg_gz / 131.068);
//    i = 0;
//    avg_gx = 0;
//    avg_gy = 0;
//    avg_gz = 0;
//  }
//  avg_gx += gx;
//  avg_gy += gy;
//  avg_gz += gz;
//  i++;
//  Serial.print("a/g:\t");
//

  
  
  Serial.print(ax / kAccelScale); Serial.print("\t");
  Serial.print(ay / kAccelScale); Serial.print("\t");
  Serial.print(az / kAccelScale); Serial.print("\t");

  double dax = ax / kAccelScale;
  double day = ay / kAccelScale;
  double daz = az / kAccelScale;
  
  double daxy = sqrt(dax*dax + day*day);
  double angle = atan(daxy/daz);
  angle = abs(angle);
  angle *= (180.0 /kPi);  

  digitalWrite(3, (int)(angle > 10));
  
  Serial.print(gx / kGyroScale + 5.16); Serial.print("\t");
  Serial.print(gy / kGyroScale + 1.65); Serial.print("\t");
  Serial.print(gz / kGyroScale + 5.79); Serial.print("\t");

  Serial.println(angle);
}
