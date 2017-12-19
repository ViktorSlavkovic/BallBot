#include "bb/motor_controller.h"
#include "util/shared_buffer.h"

#include <chrono>
#include <cstdio>
#include <functional>
#include <thread>

#include <wiringPi.h>

int main(int argc, char* argv[]) {
  wiringPiSetup();

  util::SharedBuffer<bb::MotorControlCommand> motor_controller_buffer;
  std::thread motor_controller_thread(bb::MotorController::Control,
                                      std::ref(motor_controller_buffer));
  
  bb::MotorControlCommand command;
  while (true) {
    for (int dir = 0; dir < 2; dir++) {
      for (int gear = 0; gear < 256; gear++) {
        printf("MAIN Gear: %d Dir:%d\n", gear, dir);
        for (int motor = 0; motor < 3; motor++) {
          command.dir[motor] = dir;
          command.gear[motor] = gear;
        }
        motor_controller_buffer.Push(command);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
    }
  }
  motor_controller_thread.join();
  return 0;
}
