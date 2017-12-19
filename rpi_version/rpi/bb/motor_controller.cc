#include "motor_controller.h"
#include <wiringPi.h>

#include <cstdio>
#include <cstring>
#include <thread>

namespace bb {

constexpr const int MotorController::kStepPins_[];
constexpr const int MotorController::kDirPins_[];
constexpr const int MotorController::kNumberOfSteppers_;

void MotorController::Control(SharedBuffer<MotorControlCommand>& buffer) {
  InitializeSteppers();

  MotorControlCommand command;

  std::chrono::time_point<std::chrono::system_clock> t0[3] = {
    std::chrono::system_clock::now(),
    std::chrono::system_clock::now(),
    std::chrono::system_clock::now()
  };
  
  while (true) {
    buffer.TryPop(command);
    
    auto t = std::chrono::system_clock::now();    
    bool tick = false;
    for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
      if (std::chrono::duration_cast<std::chrono::microseconds>(t - t0[i])
          .count() >= GearToMicors(command.gear[i])) {
        digitalWrite(kDirPins_[i], command.dir[i]);              
        digitalWrite(kStepPins_[i], HIGH);
        tick = true;
        t0[i] = t;
      }
    }
    
    if (tick) {
      std::this_thread::sleep_for(std::chrono::microseconds(300));
      for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
        digitalWrite(kStepPins_[i], LOW);
      }
      std::this_thread::sleep_for(std::chrono::microseconds(300));
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

constexpr uint64_t kMinGearStepMicros = 100000; // 100 ms
constexpr uint64_t kMaxGearStepMicros = 800; // 0.8 ms

uint64_t MotorController::GearToMicors(uint8_t gear) {
  if (!gear) return ~uint64_t(0);
  uint64_t step = (kMinGearStepMicros - kMaxGearStepMicros) / 254;
  return kMaxGearStepMicros + (255 - gear) * step;
}

}; //namespace bb