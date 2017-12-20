#include "bb/pid_controller.h"

#include <cstdio>

namespace bb {

void PidController::Control(
    SharedBuffer<MPUReader::Gravity>& buffer_sensor,
    SharedBuffer<MotorControlCommand>& buffer_motors) {

  MotorControlCommand command;
  uint8_t gear = 0;
  
  while (true) {
    auto gravity = buffer_sensor.Pop();
    printf("%.2f %.2f %.2f\n", gravity.x, gravity.y, gravity.z);
    gear++;
    printf("Adding command: ");
    for (int motor = 0; motor < 3; motor++) {
      command.dir[motor] = 0;
      command.gear[motor] = gear;
      printf("motor: %d gear: %d ", motor ,gear);
    }
    printf("\n");
    buffer_motors.Push(command);
  }
}

};  // namespace bb
