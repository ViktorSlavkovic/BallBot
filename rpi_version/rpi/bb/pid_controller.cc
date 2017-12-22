#include "bb/pid_controller.h"

#include <cmath>
#include <cstdio>

namespace bb {

// void PidController::Control(
//     SharedBuffer<std::string>& buffer_udp_sender,
//     SharedBuffer<MPUReader::Gravity>& buffer_sensor,
//     SharedBuffer<MotorControlCommand>& buffer_motors) {

//   MotorControlCommand command;
//   uint8_t gear = 0;
  
//   while (true) {
//     constexpr const double kPi = 3.14159265358979323846; 
//     auto gravity = buffer_sensor.Pop();
//     double gx = gravity.x;
//     double gy = gravity.y;
//     double gz = -gravity.z;
//     double gxy = sqrt(gx * gx + gy * gy);
//     // theta - in x-y plane (-PI, PI)
//     double theta = atan2(gy, gx);
//     // alpha - with z axis (0, PI)
//     double alpha = atan2(gxy, gz);
    
//     printf("%7.2f %7.2f %7.2f -> theta: %7.2f alpha: %7.2f\n",
//            gx, gy, -gz,
//            theta * 180 / kPi, alpha * 180 / kPi);
//     gear++;
//     // printf("Adding command: ");
//     for (int motor = 0; motor < 3; motor++) {
//       command.dir[motor] = 0;
//       command.gear[motor] = gear;
//       // printf("motor: %d gear: %d ", motor ,gear);
//     }
//     // printf("\n");
//     buffer_motors.Push(command);
//   }


  void PidController::Control(
    SharedBuffer<std::string>& buffer_udp_sender,
    SharedBuffer<bb::DirectionCommand>& buffer_direction){
    while (true) {
      std::string message;
      switch (buffer_direction.Pop()) {
        case DirectionCommand::FORWARD:
          message = "FORWARD received.";
          break;
        case DirectionCommand::ROTATE_LEFT:
          message = "ROTATE_RIGHT received.";
          break;
        case DirectionCommand::ROTATE_RIGHT:
          message = "ROTATE_RIGHT received.";
          break;
        case DirectionCommand::STOP:
          message = "STOP received.";
          break;
      }
      buffer_udp_sender.Push(message);
    }
  };
};  // namespace bb
