#include "bb/mpu_reader_x3.h"

#include "third_party/i2c_dev/i2c_dev.h"

#include <chrono>
#include <cstdio>
#include <thread>

#include <wiringPi.h>

namespace bb {

constexpr const int MPUReaderX3::muxSelectPins[];
constexpr const int MPUReaderX3::fifoSize;
constexpr const int MPUReaderX3::bufferLimit;
constexpr const int MPUReaderX3::mpusNumber;
MPU6050 MPUReaderX3::mpus[mpusNumber];
int MPUReaderX3::packetSize;

void MPUReaderX3::SwitchMpus(int i) {
  switch(i) {
    case 0: {
      I2Cdev::SetBus(1);
      digitalWrite(muxSelectPins[0], LOW);
      digitalWrite(muxSelectPins[1], LOW);
      break;
    }
    case 1: {
      I2Cdev::SetBus(1);
      digitalWrite(muxSelectPins[0], HIGH);
      digitalWrite(muxSelectPins[1], LOW);
      break;
    }
    case 2: {
      I2Cdev::SetBus(0);
      // digitalWrite(muxSelectPins[0], HIGH);
      // digitalWrite(muxSelectPins[1], HIGH);
      break;
    }
    default: {
        // ERROR.
        break;
    }
  }
}

void MPUReaderX3::InitializeMpus() {
  // Initialize pins for MUX.
  pinMode(muxSelectPins[0], OUTPUT);
  pinMode(muxSelectPins[1], OUTPUT);

  // Initialize MPUs.
  for (int i = 0; i < mpusNumber; i++) {
    SwitchMpus(i);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    int n_tries = 15;
    bool test_connection_ok = 0;
    int dmp_init_status = 0;

    while (n_tries--) {
      printf("\n******* Init MPU %d; %d tries left *******\n", i, n_tries);
      mpus[i].initialize();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      // If connection has not been established, report error and terminate
      // exectution.
      if (!(test_connection_ok = mpus[i].testConnection())) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      // Initialize DMP.
      if ((dmp_init_status = mpus[i].dmpInitialize()) == 0) {
        mpus[i].setDMPEnabled(true);
        packetSize = mpus[i].dmpGetFIFOPacketSize();
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      break;
    }

    if (!test_connection_ok) {
      // TODO: Report error.
      printf("\n****************** MPU testConnection on %i FAILED\n", i);
      exit(MPU_CONNECTION_FAILED);
    }

    if (dmp_init_status != 0) {
      // TODO: Report error.
      printf("\n****************** MPU dmpInitialize on %i FAILED\n", i);
      exit(MPU_DMP_INIT_FAILED);
    }
  }
}

void MPUReaderX3::Read(SharedBuffer<GravityVectors>& buffer) {
  VectorFloat gravity[3];
  int fifoCount;
  uint8_t fifoBuffer[fifoSize];
  Quaternion quaternion;		

  InitializeMpus();

  printf("\n****************** Sucessfully initialized MPUs...\n");

  while (true) {
    for (int i = 0; i < mpusNumber; i++) {
      SwitchMpus(i);
      fifoCount = mpus[i].getFIFOCount();

      if (fifoCount == bufferLimit) {
        mpus[i].resetFIFO();
      }

      // Wait for new data to be available
      while ((fifoCount = mpus[i].getFIFOCount()) < packetSize) {}
      mpus[i].getFIFOBytes(fifoBuffer, packetSize);
      mpus[i].dmpGetQuaternion(&quaternion, fifoBuffer);
      mpus[i].dmpGetGravity(&gravity[i], &quaternion);
      // printf("%d: %7.2f, %7.2f, %7.2f\n", 
      //        i, 
      //        gravity[i].x,
      //        gravity[i].y,
      //        gravity[i].z);
    }
    buffer.Push(std::make_tuple(gravity[0], gravity[1], gravity[2]));
  }
}

};  // namespace bb
