#ifndef MPU_READER_H
#define MPU_READER_H


#include "third_party/i2c_dev/i2c_dev.h"
#include "third_party/mpu_6050/mpu6050_6Axis_MotionApps20.h"
#include "shared_buffer/shared_buffer.h"

#define MPU_CONNECTION_FAILED 1
#define MPU_DMP_INIT_FAILED 2

struct position_t {
	VectorFloat positionArray[3];
};

class MPUReader {
public:

	static void Read(SharedBuffer<position_t> &sharedBuffer);

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


#endif