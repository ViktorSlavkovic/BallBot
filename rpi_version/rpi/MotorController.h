#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#define NUMBER_OF_STEPPERS 3

#include <chrono>
#include <ctime>

struct MotorControlCommand {
	uint8_t dir[NUMBER_OF_STEPPERS];
	uint8_t gear[NUMBER_OF_STEPPERS];

	MotorControlCommand(
		uint8_t dir[] = {0, 0, 0}, 
		uint8_t gear[] = {0, 0, 0});

}

class MotorController {
public:

	static void Control(SharedBuffer<MotorControlCommand> &buffer);

private:
	static const int kStepPins_[] = {25, 23, 21};
	static const int kDirPins_[] = {24, 22, 29};
	static const int kNumberOfSteppers_ = 3;

	static void InitializeSteppers();

	static uint32_t GearToMicors(uint8_t gear);
}

#endif