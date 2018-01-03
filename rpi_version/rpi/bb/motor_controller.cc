#include "motor_controller.h"
#include <wiringPi.h>

#include <cstdio>
#include <cstring>
#include <thread>

namespace bb {

constexpr const int MotorController::kStepPins_[];
constexpr const int MotorController::kDirPins_[];
constexpr const int MotorController::kNumberOfSteppers_;
constexpr const int MotorController::kSerPin;
constexpr const int MotorController::kRclkPin;
constexpr const int MotorController::kSrclkPin;
uint16_t MotorController::curr_config;


void MotorController::ShiftInBit(bool x) {
  digitalWrite(kSerPin, x ? HIGH : LOW);
  digitalWrite(kSrclkPin, HIGH);
  digitalWrite(kSrclkPin, LOW);
}

void MotorController::ShiftIn16(uint16_t x) {
  uint16_t mask = 1;
  for (int i = 0; i < 16; i++) {
    ShiftInBit(x & mask);
    mask <<= 1;
  }
  // refresh
  digitalWrite(kRclkPin, HIGH);
  digitalWrite(kRclkPin, LOW);
}

void MotorController::Control(SharedBuffer<MotorControlCommand>& buffer) {
  InitializeSteppers();

  MotorControlCommand command;

  std::chrono::time_point<std::chrono::system_clock> t0[3] = {
    std::chrono::system_clock::now(),
    std::chrono::system_clock::now(),
    std::chrono::system_clock::now()
  };
  
  while (true) {
    if (buffer.TryPop(command)) {
      SetEnabled(command.enabled);
    }
    
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
  pinMode(kSerPin, OUTPUT);
  digitalWrite(kSerPin, LOW);
  pinMode(kSrclkPin, OUTPUT);
  digitalWrite(kSrclkPin, LOW);
  pinMode(kRclkPin, OUTPUT);
  digitalWrite(kRclkPin, LOW);
  
  // TODO: Document all modes.
  // This mode is 32 microsteps per step.
  ShiftIn16(curr_config = 0x1F80);
  for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
    pinMode(kStepPins_[i], OUTPUT);
    pinMode(kDirPins_[i], OUTPUT);
    digitalWrite(kStepPins_[i], LOW);
    digitalWrite(kDirPins_[i], LOW);
  }
}

constexpr uint64_t kMinGearStepMicros = 50000; // 50 ms
constexpr uint64_t kMaxGearStepMicros = 600; // 0.6 ms

uint64_t MotorController::GearToMicors(uint8_t gear) {
  if (!gear) return ~uint64_t(0);
  uint64_t step = (kMinGearStepMicros - kMaxGearStepMicros) / 254;
  return kMaxGearStepMicros + (255 - gear) * step;
}

void MotorController::SetEnabled(const bool enabled[NUMBER_OF_STEPPERS]) {
  auto old = curr_config;
  uint16_t mask = 0x2000;
  for (int i = 0; i < NUMBER_OF_STEPPERS; i++) {
    if (enabled[i]) {
      curr_config &= (~mask); 
    } else {
      curr_config |= mask;
    }
    mask <<= 1;
  }
  if ((old & 0xE000) != (curr_config & 0xE000)) {
    ShiftIn16(curr_config);
  }
}

}; //namespace bb