#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#define NUMBER_OF_STEPPERS 3

#include "shared_buffer/shared_buffer.h"

#include <chrono>
#include <cstring>
#include <ctime>

struct MotorControlCommand {
	uint8_t dir[NUMBER_OF_STEPPERS];
	uint8_t gear[NUMBER_OF_STEPPERS];

	MotorControlCommand() {
		for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
			this->dir[i] = 0;
			this->gear[i] = 0;
		}
	}

	MotorControlCommand(const uint8_t dir[NUMBER_OF_STEPPERS],
	                    const uint8_t gear[NUMBER_OF_STEPPERS]) {
		for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
			this->dir[i] = dir[i];
			this->gear[i] = gear[i];
		}
	};

};

class MotorController {
public:

	static void Control(SharedBuffer<MotorControlCommand> &buffer);

private:
	static constexpr const int kStepPins_[] = {25, 23, 21};
	static constexpr const int kDirPins_[] = {24, 22, 29};
	static constexpr const int kNumberOfSteppers_ = 3;

	static void InitializeSteppers();

	static uint32_t GearToMicors(uint8_t gear);
};

#endif