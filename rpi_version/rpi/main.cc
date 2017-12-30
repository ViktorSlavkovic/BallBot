#include "bb/command_receiver.h"
#include "bb/floor_controller.h"
#include "bb/motor_controller.h"
#include "bb/mpu_reader.h"
#include "bb/pid_controller.h"
#include "bb/udp_sender.h"
#include "crane/crane_controller.h"
#include "util/shared_buffer.h"

#include <chrono>
#include <cstdio>
#include <exception>
#include <functional>
#include <thread>

#include <wiringPi.h>

util::SharedBuffer<bb::MotorControlCommand> motors_buffer(5);
util::SharedBuffer<bb::MPUReader::Gravity> sensors_buffer(5);

double evaluate(/* parameters */) {
  crane::CraneController::Instance().Rise();
  crane::CraneController::Instance().Drop(20);
  auto t = std::chrono::system_clock::now();
  std::this_thread::sleep_for(std::chrono::seconds(30));    
  std::thread pid_controller_thread(bb::PidController::Control,
    std::ref(sensors_buffer), std::ref(motors_buffer));
  std::this_thread::sleep_for(std::chrono::seconds(90));  
  crane::CraneController::Instance().Drop(150);
  pid_controller_thread.join();
  auto d = std::chrono::system_clock::now() - t;
  auto dsec = std::chrono::duration_cast<std::chrono::milliseconds>(d) * 0.001;
  printf("Fallen in %10.3f sec!\n", dsec);
}

int main(int argc, char* argv[]) {
  wiringPiSetup();
  // Dispatch motor controller thread.
  std::thread motor_controller_thread(bb::MotorController::Control,
                                      std::ref(motors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // Dispatch mpu reader thread.
  std::thread mpu_reader_thread(bb::MPUReader::Read, std::ref(sensors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // Evaluate once.
  crane::CraneController::Instance().Drop(20);  
  evaluate();
  crane::CraneController::Instance().Rise();
  crane::CraneController::Instance().Drop(20);

  getchar();
  std::terminate();
  return 0;
}
