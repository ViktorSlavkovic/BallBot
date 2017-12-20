#include "bb/motor_controller.h"
#include "bb/mpu_reader.h"
#include "bb/pid_controller.h"
#include "util/shared_buffer.h"

#include <chrono>
#include <cstdio>
#include <exception>
#include <functional>
#include <thread>

#include <wiringPi.h>

int main(int argc, char* argv[]) {
  wiringPiSetup();

  util::SharedBuffer<bb::MotorControlCommand> motors_buffer;
  util::SharedBuffer<bb::MPUReader::Gravity> sensors_buffer;

  std::thread motor_controller_thread(bb::MotorController::Control,
                                      std::ref(motors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::thread pid_controller_thread(bb::PidController::Control,
                                    std::ref(sensors_buffer),
                                    std::ref(motors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::thread mpu_reader_thread(bb::MPUReader::Read, std::ref(sensors_buffer));
  
  getchar();
  std::terminate();
  // printf("Terminating MPU reader thread...\n");
  // mpu_reader_thread.terminate();
  // printf("Terminating PID controller exited...\n");  
  // pid_controller_thread.join();
  // printf("Terminating motor controller exited...\n");      
  // motor_controller_thread.join();
  
  // bb::MotorControlCommand command;
  // while (true) {
  //   for (int dir = 0; dir < 2; dir++) {
  //     for (int gear = 0; gear < 256; gear++) {
  //       printf("MAIN Gear: %d Dir:%d\n", gear, dir);
  //       for (int motor = 0; motor < 3; motor++) {
  //         command.dir[motor] = dir;
  //         command.gear[motor] = gear;
  //       }
  //       motors_buffer.Push(command);
  //       std::this_thread::sleep_for(std::chrono::milliseconds(500));
  //     }
  //   }
  // }
  
  return 0;
}
