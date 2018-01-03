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
#include <cstdlib>
#include <ctime>
#include <exception>
#include <functional>
#include <thread>

#include <wiringPi.h>

util::SharedBuffer<bb::MotorControlCommand> motors_buffer(5);
util::SharedBuffer<bb::MPUReader::Gravity> sensors_buffer(5);

double evaluate(const bb::PidController::ParameterSet& params) {
  crane::CraneController::Instance().Rise();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));    
  crane::CraneController::Instance().Drop(100);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));    
  auto t = std::chrono::system_clock::now();
  std::this_thread::sleep_for(std::chrono::seconds(30));    
  std::thread pid_controller_thread(bb::PidController::Control,
    std::ref(sensors_buffer), std::ref(motors_buffer), params);
  std::this_thread::sleep_for(std::chrono::seconds(90));  
  crane::CraneController::Instance().Drop(150);
  pid_controller_thread.join();
  auto d = std::chrono::system_clock::now() - t;
  auto dsec = std::chrono::duration_cast<std::chrono::milliseconds>(d).count()
              * 0.001;
  printf("Fallen in %10.3f sec!\n", dsec);
  return dsec;
}

void test_motors() {
  crane::CraneController::Instance().Rise();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));    
  crane::CraneController::Instance().Drop(100);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));    
  bb::MotorControlCommand command;
  for (int i = 0; i < 3; i++) {
    printf("Press to test %d\n", i);
    getchar();
    for (int j = 0; j < 3; j++) {
      command.dir[j] = 0;
      command.gear[j] = 0;
    }
    command.dir[i] = 0;
    command.gear[i] = 255;
    motors_buffer.Push(command);
    std::this_thread::sleep_for(std::chrono::seconds(3));    
    command.dir[i] = 1;
    motors_buffer.Push(command);
    std::this_thread::sleep_for(std::chrono::seconds(3));    
    command.gear[i] = 0;
    motors_buffer.Push(command);    
  }
  printf("Done testing\n");
}

int main(int argc, char* argv[]) {
  srand(time(0));
  wiringPiSetup();
  // Dispatch motor controller thread.
  std::thread motor_controller_thread(bb::MotorController::Control,
                                      std::ref(motors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // Dispatch mpu reader thread.
  std::thread mpu_reader_thread(bb::MPUReader::Read, std::ref(sensors_buffer));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  // Test motors.
  // crane::CraneController::Instance().Drop(50);
  // test_motors();
  // crane::CraneController::Instance().Rise();
  // crane::CraneController::Instance().Drop(20);

  // Evaluate once.
  crane::CraneController::Instance().Drop(50);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));    
  const auto& params =  bb::PidController::ParameterSet::Generate();
  double d = evaluate(params);
  printf("%s\n", params.ToString().c_str());
  printf("   = %7.2f\n", d);
  crane::CraneController::Instance().Rise();
  crane::CraneController::Instance().Drop(20);

  getchar();
  std::terminate();
  return 0;
}
