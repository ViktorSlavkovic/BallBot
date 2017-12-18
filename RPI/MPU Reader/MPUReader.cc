#include "MPUReader.h"

void MPUReader::switchMpu(int i) {
		switch(i) {
		case 0:
			digitalWrite(muxSelectPins[0], LOW);
			digitalWrite(muxSelectPins[1], LOW);
			break;
		case 1:
			digitalWrite(muxSelectPins[0], HIGH);
			digitalWrite(muxSelectPins[1], LOW);
			break;
		case 2:
			digitalWrite(muxSelectPins[0], HIGH);
			digitalWrite(muxSelectPins[1], HIGH);
			break;
		default:
			// ERROR.
			break;
	}
}

void MPUReader::InitializeMpus() {
	// Initialize pins for MUX.
	pinMode(muxSelectPins[0], OUTPUT);
	pinMode(muxSelectPins[1], OUTPUT);

	// Initialize MPUs.
	for (int i = 0; i < mpusNumber; i++) {
		switchMpu(i);
		mpus[i].initialize();
	
		// If connection has not been established, report error and terminate exectution.
		if (!mpus[i].testConnection()) {
			// TODO: Report error.
			exit(MPU_CONNECTION_FAILED);
		}

		// Initialize DMP.
		int dmpInitStatus = mpus[i].dmpInitialize();

		if (dmpInitStatus == 0) {
			mpus[i].setDMPEnabled(true);

			packetSize = mpus[i].dmpGetFIFOPacketSize();
		} else {
			// TODO: Report error.
			exit(MPU_DMP_INIT_FAILED);
		}	
	}
}

void MPUReader::read(SharedBuffer<position_t> &sharedBuffer) {
	position_t position;
	int fifoCount;
	int fifoBuffer[fifoSize];
	Quaternion quaternion;		

	InitializeMpus();

	while (true) {
		for (int i = 0; i < mpusNumber; i++) {
			switchMpu(i);

			fifoCount = mpus[i].getFIFOCount();

			if (fifoCount == bufferLimit) {
				mpus[i].resetFIFO();
			}

			// Wait for new data to be available
			while ((fifoCount = mpus[i].getFIFOCount()) < packetSize); 
	
			mpus[i].dmpGetQuaternion(&quaternion, fifoBuffer);
			mpus[i].dmpGetGravity(&(position.positionArray[i]), &quaternion);
			
			sharedBuffer.put(position);
		}
	}
}