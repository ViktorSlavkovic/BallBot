#include "bb/motor_controller.h"
#include "bb/mpu_reader.h"
#include "bb/pid_controller.h"
#include "bb/floor_controller.h"
#include "util/shared_buffer.h"
#include "bb/command_receiver.h"

#include <chrono>
#include <cstdio>
#include <exception>
#include <functional>
#include <thread>

#include <wiringPi.h>

int main(int argc, char* argv[]) {
  wiringPiSetup();

  // util::SharedBuffer<bb::DirectionCommand> direction_buffer;
  util::SharedBuffer<bb::MotorControlCommand> motors_buffer;
  util::SharedBuffer<bb::MPUReader::Gravity> sensors_buffer;
  
  // std::thread cmd_thread(bb::CommandReceiver::Receive,
  //                        std::ref(direction_buffer),
  //                        54321);


  std::thread motor_controller_thread(bb::MotorController::Control,
                                      std::ref(motors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::thread pid_controller_thread(bb::PidController::Control,
      std::ref(sensors_buffer), std::ref(motors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // std::thread floor_thread(bb::FloorController::Control,
  //                                   std::ref(direction_buffer),
  //                                   std::ref(motors_buffer));
  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::thread mpu_reader_thread(bb::MPUReader::Read, std::ref(sensors_buffer));
  
  getchar();
  std::terminate();
  // // printf("Terminating MPU reader thread...\n");
  // // mpu_reader_thread.terminate();
  // // printf("Terminating PID controller exited...\n");  
  // // pid_controller_thread.join();
  // // printf("Terminating motor controller exited...\n");      
  // // motor_controller_thread.join();
  
  // bb::MotorControlCommand command;

  // for (int motor = 0; motor < 3; motor++) {
  //   command.dir[motor] = 0;
  //   command.gear[motor] = 1;
  // }
  // motors_buffer.Push(command);
  // std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // getchar();
  // std::terminate();
  
  // // while (true) {
  // //   for (int dir = 0; dir < 2; dir++) {
  // //     for (int gear = 0; gear < 256; gear++) {
  // //       printf("MAIN Gear: %d Dir:%d\n", gear, dir);
  // //       for (int motor = 0; motor < 3; motor++) {
  // //         command.dir[motor] = dir;
  // //         command.gear[motor] = gear;
  // //       }
  // //       motors_buffer.Push(command);
  // //       std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // //     }
  // //   }
  // // }
  
  return 0;
}
