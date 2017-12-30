#include "bb/mpu_reader.h"

#include "third_party/i2c_dev/i2c_dev.h"

#include <chrono>
#include <cstdio>
#include <thread>

namespace bb {

constexpr const int MPUReader::fifoSize;
constexpr const int MPUReader::bufferLimit;
MPU6050 MPUReader::mpu;
int MPUReader::packetSize;

void MPUReader::InitializeMpu() {
  // Initialize MPU.

  int n_tries = 15;
  bool test_connection_ok = 0;
  int dmp_init_status = 0;

  while (n_tries--) {
    printf("\n******* Init MPU; %d tries left *******\n", n_tries);
    mpu.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // If connection has not been established, report error and terminate
    // exectution.
    if (!(test_connection_ok = mpu.testConnection())) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    // Initialize DMP.
    if ((dmp_init_status = mpu.dmpInitialize()) == 0) {
      mpu.setDMPEnabled(true);
      packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    break;
  }

  if (!test_connection_ok) {
    // TODO: Report error.
    printf("\n******* MPU testConnection FAILED *******\n");
    exit(MPU_CONNECTION_FAILED);
  }

  if (dmp_init_status != 0) {
    // TODO: Report error.
    printf("\n******* MPU dmpInitialize FAILED *******\n");
    exit(MPU_DMP_INIT_FAILED);
  }
}

void MPUReader::Read(SharedBuffer<Gravity>& buffer) {
  VectorFloat gravity;
  int fifoCount;
  uint8_t fifoBuffer[fifoSize];
  Quaternion quaternion;		

  InitializeMpu();

  printf("\n******* Sucessfully initialized MPU *******\n");

  while (true) {
    fifoCount = mpu.getFIFOCount();

    if (fifoCount == bufferLimit) {
      printf("Overflow!\n");
      mpu.resetFIFO();
    }
again:
    // Wait for new data to be available
    while ((fifoCount = mpu.getFIFOCount()) < packetSize) {}
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    mpu.dmpGetQuaternion(&quaternion, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &quaternion);
    if (gravity.getMagnitude() > 1.1) {
      printf("Bad gravity!\n");
      mpu.resetFIFO();
      goto again;
    }
    // printf("%d: %7.2f, %7.2f, %7.2f\n", 
    //        gravity[i].y,
    //        gravity[i].z);
    buffer.Push(gravity);
  }
}

};  // namespace bb
