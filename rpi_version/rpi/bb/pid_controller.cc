#include "bb/pid_controller.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace bb {

static constexpr const double kPi = 3.14159265358979323846; 
static constexpr const double kMotorPos[] = {
  145 * kPi / 180,
   25 * kPi / 180,
  -95 * kPi / 180
};

static constexpr const int kMotorInDir[] = { 0, 0, 0 };
static constexpr const int kMotorOutDir[] = { 1, 1, 1 };
static constexpr const double kPushAngleThreshold = kPi / 12;
static constexpr const double kFallenAngleThreshold = kPi / 6;
static constexpr const int kSteerLeftDir = 0;
static constexpr const int kSteerRightDir = 1;
static constexpr const int kTopSteerGear = 150;

// x in (-pi, pi)
inline double angle_across(double x) {
  x += kPi;
  if (x > kPi) {
    x -= 2 * kPi;
  }
  return x;
}

// from, to in (-pi, pi)
inline double angle_left_dist(double from, double to) {
  if (from > to) return from - to;
  from += 2 * kPi;
  return from - to;
}

inline double angle_right_dist(double from, double to) {
  if (from < to) return to - from;
  to += 2 * kPi;
  return to - from;
}

// x, y in (-pi, pi)
inline double angle_dist(double x, double y) {
  return std::min(angle_left_dist(x, y), angle_right_dist(x, y));
}

inline bool motor_on_the_right(int motor_from, int motor_subj) {
  return angle_left_dist(kMotorPos[motor_from], kMotorPos[motor_subj]) >
         angle_right_dist(kMotorPos[motor_from], kMotorPos[motor_subj]);
}

void PidController::Control(
    SharedBuffer<MPUReader::Gravity>& buffer_sensor,
    SharedBuffer<MotorControlCommand>& buffer_motors) {

  MotorControlCommand command;

  while (true) {

    auto gravity = buffer_sensor.Pop();
    double gx = gravity.x;
    double gy = gravity.y;
    double gz = -gravity.z;
    double gxy = sqrt(gx * gx + gy * gy);
    // theta - in x-y plane (-PI, PI)
    double theta = atan2(gy, gx);
    // alpha - with z axis (0, PI)
    double alpha = atan2(gxy, gz);

    double low = theta;
    double high = angle_across(low);

    printf("%7.2f %7.2f %7.2f -> theta: %7.2f alpha: %7.2f\n",
           gx, gy, -gz,
           theta * 180 / kPi, alpha * 180 / kPi);

    // Choosing gears...

    // 1) If fallen - stop all motors.
    if (alpha > kFallenAngleThreshold) {
      for (int motor = 0; motor < 3; motor++) {
        command.dir[motor] = 0;
        command.gear[motor] = 0;
      }
      buffer_motors.Push(command);
      continue;
    }
    // 2) Move the lower two wheels towards the interior (push) if the lowest
    //    point is not too close to some of the wheels, otherwise move th
    //    higher two wheels towards the exterior (pull).
    struct dists {
      int idx;
      double dist_low;
      double dist_high;
    };
    dists motor_dists[3];
    for (int i = 0; i < 3; i++) {
      motor_dists[i].idx = i;
      motor_dists[i].dist_low = angle_dist(low, kMotorPos[i]);
      motor_dists[i].dist_high = angle_dist(high, kMotorPos[i]);
    }
    std::sort(motor_dists, motor_dists + 3,
              [](const dists& lhs, const dists& rhs) -> bool {
      return lhs.dist_low < rhs.dist_low;
    });
    // for (int i = 0; i < 3; i++) {
    //   printf("| %d %7.2f ", motor_dists[i].idx, motor_dists[i].dist_low * 180 / kPi);
    // }
    // printf("\n");
    if (motor_dists[0].dist_low > kPushAngleThreshold) {
      // Find push directions.
      command.dir[motor_dists[0].idx] =
          motor_on_the_right(motor_dists[0].idx, motor_dists[1].idx)
          ? kSteerRightDir
          : kSteerLeftDir;
      command.dir[motor_dists[1].idx] = 1 - command.dir[motor_dists[0].idx];
      // Caclute pushing gears.
      command.gear[motor_dists[0].idx] =
          round(alpha / kFallenAngleThreshold * 255);
      command.gear[motor_dists[1].idx] =
          round(alpha / kFallenAngleThreshold * 255/* *
                motor_dists[0].dist_low / motor_dists[1].dist_low*/);
      // Calculate steer direction.
      double mhigh_left_dist =
          angle_left_dist(kMotorPos[motor_dists[2].idx], high);
      double mhigh_right_dist =
          angle_right_dist(kMotorPos[motor_dists[2].idx], high);
      if (mhigh_left_dist > mhigh_right_dist) {
        command.dir[motor_dists[2].idx] = kSteerRightDir;
      } else {
        command.dir[motor_dists[2].idx] = kSteerLeftDir;
      }
      // Calculate steering gear.
      command.gear[motor_dists[2].idx] = round(
          motor_dists[2].dist_high / kPushAngleThreshold * kTopSteerGear);
    } else {
      // Find pull directions.
      command.dir[motor_dists[1].idx] =
          motor_on_the_right(motor_dists[1].idx, motor_dists[2].idx)
          ? kSteerLeftDir
          : kSteerRightDir;
      command.dir[motor_dists[2].idx] = 1 - command.dir[motor_dists[1].idx];
      // Caclute pulling gears.
      command.gear[motor_dists[1].idx] =
          round(alpha / kFallenAngleThreshold * 255);
      command.gear[motor_dists[2].idx] =
          round(alpha / kFallenAngleThreshold/* * 255 *
                motor_dists[2].dist_high / motor_dists[1].dist_high*/);
      // Calculate steer direction.
      double mlow_left_dist =
          angle_left_dist(kMotorPos[motor_dists[0].idx], low);
      double mlow_right_dist =
          angle_right_dist(kMotorPos[motor_dists[0].idx], low);
      if (mlow_left_dist > mlow_right_dist) {
        command.dir[motor_dists[0].idx] = kSteerRightDir;
      } else {
        command.dir[motor_dists[0].idx] = kSteerLeftDir;
      }
      // Calculate steering gear.
      command.gear[motor_dists[0].idx] = round(
          motor_dists[0].dist_low / kPushAngleThreshold * kTopSteerGear);
    }
    buffer_motors.Push(command);
  }
}

  // void PidController::Control(
  //   SharedBuffer<std::string>& buffer_udp_sender,
  //   SharedBuffer<bb::DirectionCommand>& buffer_direction){
  //   while (true) {
  //     std::string message;
  //     switch (buffer_direction.Pop()) {
  //       case DirectionCommand::FORWARD:
  //         message = "FORWARD received.";
  //         break;
  //       case DirectionCommand::ROTATE_LEFT:
  //         message = "ROTATE_RIGHT received.";
  //         break;
  //       case DirectionCommand::ROTATE_RIGHT:
  //         message = "ROTATE_RIGHT received.";
  //         break;
  //       case DirectionCommand::STOP:
  //         message = "STOP received.";
  //         break;
  //     }
  //     buffer_udp_sender.Push(message);
  //   }
  // };
};  // namespace bb
