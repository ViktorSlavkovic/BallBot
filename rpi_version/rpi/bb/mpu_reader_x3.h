#ifndef BB_MPU_READER_X3_H_
#define BB_MPU_READER_X3_H_

#include "third_party/i2c_dev/i2c_dev.h"
#include "third_party/mpu_6050/mpu6050.h"
#include "util/shared_buffer.h"

#include <tuple>

#define MPU_CONNECTION_FAILED 1
#define MPU_DMP_INIT_FAILED 2

namespace bb {

using util::SharedBuffer;

class MPUReaderX3 {
 public:
  using GravityVectors = std::tuple<VectorFloat, VectorFloat, VectorFloat>;
  static void Read(SharedBuffer<GravityVectors>& buffer);
 private:
  static constexpr const int muxSelectPins[] = {0, 7};
  static constexpr const int fifoSize = 64;
  static constexpr const int bufferLimit = 1024;
  static constexpr const int mpusNumber = 3;
  static MPU6050 mpus[mpusNumber];
  static int packetSize;

  static void SwitchMpus(int i);

  static void InitializeMpus();
};

};  // namespace bb

#endif  // BB_MPU_READER_X3_H_