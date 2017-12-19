#include "motor_controller.h"
#include <wiringPi.h>

constexpr const int MotorController::kStepPins_[];
constexpr const int MotorController::kDirPins_[];
constexpr const int MotorController::kNumberOfSteppers_;

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
			if (std::chrono::duration_cast<std::chrono::microseconds>
				(start - std::chrono::system_clock::now()).count()
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
   		pinMode(kStepPins_[i], OUTPUT);
   		pinMode(kDirPins_[i], OUTPUT);
   		digitalWrite(kStepPins_[i], LOW);
   		digitalWrite(kDirPins_[i], LOW);
	}
}

uint32_t MotorController::GearToMicors(uint8_t gear) {
	if (!gear) return ~uint64_t(0);
	return (1 + (63 - gear) * 64llu) * 100;
}