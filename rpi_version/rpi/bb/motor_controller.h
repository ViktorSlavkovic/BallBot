#ifndef BB_MOTOR_CONTROLLER_H_
#define BB_MOTOR_CONTROLLER_H_

#define NUMBER_OF_STEPPERS 3

#include "util/shared_buffer.h"

namespace bb {

using util::SharedBuffer;

struct MotorControlCommand {
  uint8_t dir[NUMBER_OF_STEPPERS];
  uint8_t gear[NUMBER_OF_STEPPERS];
  bool enabled[3];

  MotorControlCommand() {
    for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
      this->dir[i] = 0;
      this->gear[i] = 0;
      this->enabled[i] = true;
    }
  }

  MotorControlCommand(const uint8_t dir[NUMBER_OF_STEPPERS],
                      const uint8_t gear[NUMBER_OF_STEPPERS],
                      const bool enabled[NUMBER_OF_STEPPERS]) {
    for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
      this->dir[i] = dir[i];
      this->gear[i] = gear[i];
      this->enabled[i] = enabled[i];
    }
  }
};

class MotorController {
public:

  static void Control(SharedBuffer<MotorControlCommand> &buffer);

private:
  // TODO: Move the constants to cc.
  static constexpr const int kStepPins_[] = {24, 22, 1};
  static constexpr const int kDirPins_[] = {25, 23, 21};
  static constexpr const int kNumberOfSteppers_ = 3;

  static constexpr const int kSerPin = 7;
  static constexpr const int kRclkPin = 0;
  static constexpr const int kSrclkPin = 2;
  
  static uint16_t curr_config;
  static void ShiftInBit(bool x); 
  static void ShiftIn16(uint16_t x);
  static void SetEnabled(const bool enabled[NUMBER_OF_STEPPERS]);
  static void InitializeSteppers();
  static uint64_t GearToMicors(uint8_t gear);
};

};  // namespace bb

#endif  // BB_MOTOR_CONTROLLER_H_