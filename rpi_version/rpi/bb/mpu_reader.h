#ifndef BB_MPU_READER_H_
#define BB_MPU_READER_H_

#include "third_party/mpu_6050/mpu6050.h"
#include "util/shared_buffer.h"

#define MPU_CONNECTION_FAILED 1
#define MPU_DMP_INIT_FAILED 2

namespace bb {

using util::SharedBuffer;

class MPUReader {
 public:
  using Gravity = VectorFloat;
  static void Read(SharedBuffer<Gravity>& buffer);
 private:
  static constexpr const int fifoSize = 64;
  static constexpr const int bufferLimit = 1024;
  static MPU6050 mpu;
  static int packetSize;

  static void InitializeMpu();
};

};  // namespace bb

#endif  // BB_MPU_READER_H_