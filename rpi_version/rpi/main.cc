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
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>
#include <thread>
#include <unordered_map>

#include <wiringPi.h>

util::SharedBuffer<bb::MotorControlCommand> motors_buffer(5);
util::SharedBuffer<bb::MPUReader::Gravity> sensors_buffer(5);

double evaluate(const bb::PidController::ParameterSet& params) {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  crane::CraneController::Instance().Rise();
  std::this_thread::sleep_for(std::chrono::seconds(8));
  crane::CraneController::Instance().Drop(300);
  auto t = std::chrono::system_clock::now();
  std::this_thread::sleep_for(std::chrono::seconds(30));    
  std::thread pid_controller_thread(bb::PidController::Control,
    std::ref(sensors_buffer), std::ref(motors_buffer), params);
  std::this_thread::sleep_for(std::chrono::seconds(90));  
  crane::CraneController::Instance().Drop(2100);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  crane::CraneController::Instance().ControlTreadmill(true /*start*/);
  pid_controller_thread.join();
  auto d = std::chrono::system_clock::now() - t;
  crane::CraneController::Instance().ControlTreadmill(false /*stop*/);
  std::this_thread::sleep_for(std::chrono::seconds(3));
  auto dsec = std::chrono::duration_cast<std::chrono::milliseconds>(d).count()
              * 0.001;
  printf("Fallen in %10.3f sec!\n", dsec);
  return dsec;
}

void test_motors() {
  std::this_thread::sleep_for(std::chrono::seconds(3));  
  crane::CraneController::Instance().Rise();
  std::this_thread::sleep_for(std::chrono::seconds(3));    
  crane::CraneController::Instance().Drop(500);
  std::this_thread::sleep_for(std::chrono::seconds(3));    
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

std::unordered_map<std::string, double> used_params;

void load_used_params(const std::string& path) {
  
  std::ifstream fin(path);

  if (!fin.is_open()) {
    fprintf(stderr, "Failed to open params storage file: %s.\n", path.c_str());
    fprintf(stderr, "Coninuing with an empty memory.\n");
    return;
  }

  int n; fin >> n;
  while (n--) {
    bb::PidController::ParameterSet ps; fin >> ps;
    double res; fin >> res;
    used_params[ps] = res;
  }

  fin.close();
}

void store_used_params(const std::string& path) {
  std::ofstream fout(path);

  if (!fout.is_open()) {
    fprintf(stderr, "Failed to open params storage file: %s.\n", path.c_str());
    exit(-1);
  }

  fout.precision(2);
  fout << used_params.size() << std::endl;
  for (auto p : used_params) {
    fout << bb::PidController::ParameterSet(p.first);
    fout << std::fixed << std::setprecision(5) << p.second << std::endl;
  }

  fout.close();
}

bb::PidController::ParameterSet get_best_params() {
  std::string best = "";
  double best_val = -1.0e10;
  for (const auto& p : used_params) {
    if (p.second > best_val) {
      best = p.first;
      best_val = p.second;
    }
  }
  if (best == "") {
    return bb::PidController::ParameterSet::Generate();
  }
  printf("\nRunning with best known params - duration: %.3f.\n", best_val);
  return bb::PidController::ParameterSet(best);
}

int main(int argc, char* argv[]) {
  
  if (argc < 2) {
    fprintf(stderr, "No params storage file specified.\n");
    return -1;
  }

  // Load used param map.
  load_used_params(argv[1]);
  
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
  bool run_best = argc >= 3 && std::string(argv[2]) == "best";
  const auto& params = run_best
                     ? get_best_params()
                     : bb::PidController::ParameterSet::Generate();
  double d = evaluate(params);
  printf("%s\n", params.ToString().c_str());
  printf("   = %7.2f\n", d);

  crane::CraneController::Instance().Rise();
  std::this_thread::sleep_for(std::chrono::seconds(8));
  crane::CraneController::Instance().Drop(300);
  std::this_thread::sleep_for(std::chrono::seconds(3));

  std::string params_hash = params;
  if (used_params.find(params_hash) == used_params.end()) {
    used_params[params_hash] = d;
  } else {
    used_params[params_hash] = std::min(used_params[params_hash], d);
  }

  printf("\nStoring the evaluation and exiting...\n");
  //printf("\nPress any key to store the evaluation and exit...\n");
  //getchar();

  // Store used param map.
  store_used_params(argv[1]);

  std::terminate();
  return 0;
}
