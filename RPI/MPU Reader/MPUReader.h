#ifndef MPU_READER_H
#define MPU_READER_H


#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>

#define MPU_CONNECTION_FAILED 1;
#define MPU_DMP_INIT_FAILED 2;

struct position_t {
	VectorFloat[3] positionArray;
}

class MPUReader {
	static MPU6050 mpus[mpusNumber];
	static int packetSize;
	static const int mpusNumber 3;
	static const int muxSelectPins[] = {0, 7};
	static const int fifoSize = 64;
	static const int bufferLimit = 1024;

	static void switchMpu(int i);

	static void InitializeMpus();
public:

	static void read(SharedBuffer<position_t> &sharedBuffer);
};


#endif