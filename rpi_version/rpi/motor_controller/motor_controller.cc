#include "motor_controller/motor_controller.h"

#include <wiringPi.h>

MotorControlCommand::MotorControlCommand(
	uint8_t dir[] = {0, 0, 0}, 
	uint8_t gear[] = {0, 0, 0}) {
		this->dir = dir;
		this->gear = gear;
}

void MotorController::Control(
    SharedBuffer<MotorControlCommand> &buffer) {
	InitializeSteppers();
	
	MotorControlCommand command;
	
	auto start = std::chrono::system_clock::now();
	while (true) {
		if (buffer.Try_pop(command)) {
			start = std::chrono::system_clock::now();
		};
		
		for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
			if (chrono::duration_cast<chrono::microseconds>
				(start - std::chrono::system_clock::now()) 
					>= GearToMicors(command.gear[i])) {
				digitalWrite(kStepPins_[i], HIGH);
				digitalWrite(kDirPins_[i], command.dir[i]);
			}
		}

		for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
    		digitalWrite(kDirPins_[i], LOW);
		}
	}
}

void MotorController::InitializeSteppers() {
	for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
   		pinMode(kStepPins[i], OUTPUT);
   		pinMode(kDirPins[i], OUTPUT);
   		digitalWrite(kStepPins[i], LOW);
   		digitalWrite(kDirPins[i], LOW);
	}
}

uint32_t MotorController::GearToMicors(uint8_t gear) {
	if (!gear) return ~uint64_t(0);
	return (1 + (63 - gear) * 64llu) * 100;
}