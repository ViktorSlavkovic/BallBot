#include "bb/pid_controller.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace bb {

using std::min;

static constexpr const double kMotorPos[] = {
  145 * kPi / 180,
   25 * kPi / 180,
  -95 * kPi / 180
};
static constexpr const double kFallenAngleThreshold = kPi / 180 * 12;
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
    SharedBuffer<MotorControlCommand>& buffer_motors,
    const ParameterSet& params) {
  // The reaction command passed to the motor controller.
  MotorControlCommand command;
  // 0 - calming down
  // 1 - calibration
  // 2 - reacting
  int state = 0;
  // Number of readings wasted in calming down phase.
  int num_calm_down = 3000;
  // Number of readings used for calibration.
  int num_calibrate = 1000;
  // Calibration reading index in [0, num_calibrate - 1], used to build up
  // the calibation offsets.
  int idx_calibrate = 0;
  // Used for actions on state transition such as printing the state info.
  bool state_transition = true;
  // Calibration offsets.
  double ogx = 0;
  double ogy = 0;
  double ogz = 0;
  
  while (true) {
    // 0) Sensor reading
    //
    // Read (blocking) gravity vector from MPU reader and translate it to
    // (alpha, theta) pair, where:
    //   alpha - the angle (in radians) between the robot central vertical axis
    //           and the world z axis (ball vertical central axis), bascially
    //           Pi - polar angle of the robot's MPU poing in ball's spherical
    //           corrdinate system.
    //   theta - the angle (in radians) between -Pi and Pi, representing the 
    //           position of the HIGHEST point on the robot's base disk. The
    //           zero position doesn't matter, everything is relative to motor
    //           positions in kMotorPos[].
    auto gravity = buffer_sensor.Pop();
    double gx = gravity.x;
    double gy = gravity.y;
    double gz = -gravity.z;
    if (state == 2) {
      gx -= ogx;
      gy -= ogy;
      gz -= ogz;
    }
    double gxy = sqrt(gx * gx + gy * gy);
    double theta = atan2(gy, gx);
    double alpha = atan2(gxy, gz);
    // Highest point on the robot's base disk - theta.
    double high = theta;
    // Lowest point on the robot's base disk - the oppoiste of theta.
    double low = angle_across(high);

    // 1) Handle calming down and calibration states.
    switch (state) {
      case 0: {
        if (state_transition) {
          printf("Calming down...\n");
          state_transition = false;
        }
        if (num_calm_down-- <= 0) {
          state_transition = true;
          state = 1;
        }
        continue;
      }
      case 1: {
        if (state_transition) {
          printf("Calibrating...\n");
          state_transition = false;
        }
        if (num_calibrate-- <= 0) {
          printf("Done calibrating - ogx: %7.2f ogy: %7.2f ogz: %7.2f\n",
                 ogx, ogy, ogz);
          ogz = ogz - 1.0;
          state_transition = true;
          state = 2;
        }
        if (idx_calibrate == 0) {
          ogx = gx;
          ogy = gy;
          ogz = gz;
          idx_calibrate++;
          continue;
        }
        ogx *= (double) idx_calibrate / (idx_calibrate + 1.0);
        ogy *= (double) idx_calibrate / (idx_calibrate + 1.0);
        ogz *= (double) idx_calibrate / (idx_calibrate + 1.0);
        idx_calibrate++;
        ogx += gx / idx_calibrate;
        ogy += gy / idx_calibrate;
        ogz += gz / idx_calibrate;
        continue;
      }
      case 2: {
        if (state_transition) {
          printf("Running...\n");
          state_transition = false;
        }
        break;
      }
    }

    // 2) Handle reaction state - the actual balancing.
    
    // printf("%7.2f %7.2f %7.2f -> theta: %7.2f alpha: %7.2f\t",
    //        gx, gy, -gz, theta * 180 / kPi, alpha * 180 / kPi);
    
    //   2.1) If fallen - stop all motors and return.
    if (alpha > kFallenAngleThreshold) {
      for (int motor = 0; motor < 3; motor++) {
        command.dir[motor] = 0;
        command.gear[motor] = 0;
      }
      buffer_motors.Push(command);
      printf("Falling\n");
      return;
    }

    //   2.2) Move the lower two wheels towards the interior (push) if the
    //        lowest point is not too close to some of the wheels, otherwise
    //        move the higher two wheels towards the exterior (pull).
    
    struct dists {
      // Motor index.
      int idx;
      // Motor angular distance from the lowest point on the robot's base disk.
      double dist_low;
      // Motor angular distance from the highest point on the robot's base disk.
      double dist_high;
    };
    dists motor_dists[3];
    for (int i = 0; i < 3; i++) {
      motor_dists[i].idx = i;
      motor_dists[i].dist_low = angle_dist(low, kMotorPos[i]);
      motor_dists[i].dist_high = angle_dist(high, kMotorPos[i]);
    }
    // Sort the motors by their dist_low's.
    std::sort(motor_dists, motor_dists + 3,
              [](const dists& lhs, const dists& rhs) -> bool {
      return lhs.dist_low < rhs.dist_low;
    });
    
    // // Sanity logging.
    // for (int i = 0; i < 3; i++) {
    //   printf("| %d %7.2f ",
    //          motor_dists[i].idx,
    //          motor_dists[i].dist_low * 180 / kPi);
    // }
    // printf("\n");

    // Are we pulling or pushing?

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// PUSHING /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    if (motor_dists[0].dist_low > params.push_angle_threshold) {
      
      command.enabled[motor_dists[0].idx] = true;      
      command.enabled[motor_dists[1].idx] = true;
      command.enabled[motor_dists[2].idx] = params.steer;
      
      // Find push directions.
      command.dir[motor_dists[0].idx] =
          motor_on_the_right(motor_dists[0].idx, motor_dists[1].idx)
          ? kSteerRightDir
          : kSteerLeftDir;
      command.dir[motor_dists[1].idx] = 1 - command.dir[motor_dists[0].idx];

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

      // * dist low najdonjeg ide od threshold (kad je najdodnji - on sam, od
      //   donje tacke na thershold), do PI/3 (kad je na PI/3)
      //     = ovaj gura najjace, da ispravi;
      //       kako opada, raste brzina
      // * dist low srednjeg ide od PI/3 + (PI/3 - threshold) (kad je najdonji
      //   od donje tacke na threshold), do PI/3  (kad je i najdonji od donje
      //   tacke na PI/3, zapravo isti su)
      //     = ovaj gura slabije;
      //       kako opada, raste brzina;
      //       max brzina ovde (oba na PI/3) == min brzina iznad!!!
      // * dist high za steer ide od 0 (kad je najdonji na PI/3), do 
      //   PI/3 - threshold (kad je najdonji threshold)
      //     = kako raste, raste brzina
      
      // BITNO: kad skaliramo na [0, 1] srednji i najdonji su isti!

      // low_dist to [0, 1] via division and sin
      double low_dist_lin_scale =
          1.0 -  // flip!
          min(1.0, (motor_dists[0].dist_low - params.push_angle_threshold) /
                   (kPi / 3 - params.push_angle_threshold));
      double low_dist_sin_scale = sin(kPi / 2 * low_dist_lin_scale);
      
      // high_dist of the steering wheel to [0, 1] via division and sin
      double high_dist_lin_scale_steer =
          min(1.0, motor_dists[2].dist_high /
                   (kPi / 3 - params.push_angle_threshold));
      double high_dist_sin_scale_steer =
          sin(kPi / 2 * high_dist_lin_scale_steer);
      double high_dist_cos_scale_steer =
          cos(kPi / 2 * high_dist_lin_scale_steer);  // inv

      // alpha to [0, 1] via division and sin
      double alpha_lin_scale = min(1.0, alpha / kFallenAngleThreshold);
      double alpha_sin_scale = sin(kPi / 2 * alpha_lin_scale);
      
      // Combine scales for each wheel individually.
      double alphas_lowest =
          params.push_lin_importance_lowest * alpha_lin_scale +
          (1 - params.push_lin_importance_lowest) * alpha_sin_scale;
      double alphas_middle =
          params.push_lin_importance_middle * alpha_lin_scale +
          (1 - params.push_lin_importance_middle) * alpha_sin_scale;
      double alphas_steer =
          params.push_lin_importance_steer * alpha_lin_scale +
          (1 - params.push_lin_importance_steer) * alpha_sin_scale;
      
      double low_dists_lowest =
          params.push_lin_importance_lowest * low_dist_lin_scale +
          (1 - params.push_lin_importance_lowest) * low_dist_sin_scale;
      double low_dists_middle = 
          params.push_product_importance_middle * low_dist_lin_scale +
          (1 - params.push_product_importance_middle) * low_dist_sin_scale;

      double high_dists_steer =
          params.push_lin_importance_steer * high_dist_lin_scale_steer +
          (1 - params.push_lin_importance_steer) * high_dist_sin_scale_steer;
      
      // Compute normalized gears.
      double lowest_gear_01 =
          params.push_product_importance_lowest * (
            alphas_lowest * low_dists_lowest
          ) + (1 - params.push_product_importance_lowest) * (
            params.push_alpha_importance_lowest * alphas_lowest +
            (1 - params.push_alpha_importance_lowest) * low_dists_lowest
          );

      double middle_gear_01 =
          params.push_product_importance_middle * (
            alphas_middle * low_dists_middle
          ) + (1 - params.push_product_importance_middle) * (
            params.push_alpha_importance_middle * alphas_middle +
            (1 - params.push_alpha_importance_middle) * low_dists_middle
          );
      
      double steer_gear_01 =
          params.push_product_importance_steer * (
            alphas_steer * high_dists_steer
          ) + (1 - params.push_product_importance_steer) * (
            params.push_alpha_importance_steer * alphas_steer +
            (1 - params.push_alpha_importance_steer) * high_dists_steer
          );
      
      // Compute pushing gears with offsets and right scale.
      command.gear[motor_dists[0].idx] =
          params.push_min_lowest_gear +
          round((params.push_max_lowest_gear - params.push_min_lowest_gear) * 
                lowest_gear_01);

      command.gear[motor_dists[1].idx] =
          params.push_min_middle_gear + 
          round((params.push_max_middle_gear - params.push_min_middle_gear) * 
                middle_gear_01);
      
      // Calculate steering gear with offset and right scale.
      command.gear[motor_dists[2].idx] =
          params.push_min_steer_gear +
          round((params.push_max_steer_gear - params.push_min_steer_gear) *
                steer_gear_01);
    
////////////////////////////////////////////////////////////////////////////////
    } else {  //////////////// PULLING /////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
      command.enabled[motor_dists[0].idx] = params.steer;      
      command.enabled[motor_dists[1].idx] = true;
      command.enabled[motor_dists[2].idx] = true;

      // Find pull directions.
      command.dir[motor_dists[1].idx] =
          motor_on_the_right(motor_dists[1].idx, motor_dists[2].idx)
          ? kSteerLeftDir
          : kSteerRightDir;
      command.dir[motor_dists[2].idx] = 1 - command.dir[motor_dists[1].idx];

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

      // * dist high najgornjeg ide od PI/3 (kad je najdodnji od donje tacke 0,
      //   gornji isti!),
      //   do PI/3 - threshold (kad je najdodnje od done tacke na threshold)?
      //     = ovaj gura slabije;
      //       kako raste, brzina raste
      // * dist high srednjeg ide od PI/3 (kad je najdonji od dnje tacke 0,
      //   gornji isti!),
      //   do PI/3 + threshold (kad je najdonji od donje tacke na threshold)
      //     = ovaj gura najjace da ispravi;
      //       kako raste, brzina raste,
      //       min brzina ovoga je kao max ovog iznad!!!
      // * dist low za steer ide od 0 (kad je najdonji - on sam, na 0), do 
      //   threshold (kad je na threshold)
      //     = kako raste, brzina raste

      // BITNO: kad skaliramo na [0, 1] srednji i najgornji su isti!
      
      // high_dist to [0, 1] via division and sin
      double high_dist_lin_scale =
        min(1.0,
            (motor_dists[2].dist_high - (kPi / 3 - params.push_angle_threshold))
            / params.push_angle_threshold);
      double high_dist_sin_scale = sin(kPi / 2 * high_dist_lin_scale);
      
      // low _dist of the steering wheel to [0, 1] via division and sin
      double low_dist_lin_scale_steer =
          min(1.0, motor_dists[0].dist_low / params.push_angle_threshold);
      double low_dist_sin_scale_steer = sin(kPi / 2 * low_dist_lin_scale_steer);

      // alpha to [0, 1] via division and sin
      double alpha_lin_scale = min(1.0, alpha / kFallenAngleThreshold);
      double alpha_sin_scale = sin(kPi / 2 * alpha_lin_scale);

      // Combine scales for each wheel individually.
      double alphas_highest =
          params.pull_lin_importance_highest * alpha_lin_scale +
          (1 - params.pull_lin_importance_highest) * alpha_sin_scale;
      double alphas_middle =
          params.pull_lin_importance_middle * alpha_lin_scale +
          (1 - params.pull_lin_importance_middle) * alpha_sin_scale;
      double alphas_steer =
          params.pull_lin_importance_steer * alpha_lin_scale +
          (1 - params.pull_lin_importance_steer) * alpha_sin_scale;
      
      double high_dists_highest =
          params.pull_lin_importance_highest * high_dist_lin_scale +
          (1 - params.pull_lin_importance_highest) * high_dist_sin_scale;
      double high_dists_middle = 
          params.pull_product_importance_middle * high_dist_lin_scale +
          (1 - params.pull_product_importance_middle) * high_dist_sin_scale;

      double low_dists_steer =
          params.pull_lin_importance_steer * low_dist_lin_scale_steer +
          (1 - params.pull_lin_importance_steer) * low_dist_sin_scale_steer;
      
      // Compute normalized gears.
      double highest_gear_01 =
          params.pull_product_importance_highest * (
            alphas_highest * high_dists_highest
          ) + (1 - params.pull_product_importance_highest) * (
            params.pull_alpha_importance_highest * alphas_highest +
            (1 - params.pull_alpha_importance_highest) * high_dists_highest
          );

      double middle_gear_01 =
          params.pull_product_importance_middle * (
            alphas_middle * high_dists_middle
          ) + (1 - params.pull_product_importance_middle) * (
            params.pull_alpha_importance_middle * alphas_middle +
            (1 - params.pull_alpha_importance_middle) * high_dists_middle
          );
      
      double steer_gear_01 =
          params.pull_product_importance_steer * (
            alphas_steer * low_dists_steer
          ) + (1 - params.pull_product_importance_steer) * (
            params.pull_alpha_importance_steer * alphas_steer +
            (1 - params.pull_alpha_importance_steer) * low_dists_steer
          );
      
      // Compute pushing gears with offsets and right scale.
      command.gear[motor_dists[0].idx] =
          params.pull_min_highest_gear +
          round((params.pull_max_highest_gear - params.pull_min_highest_gear) * 
                highest_gear_01);

      command.gear[motor_dists[1].idx] =
          params.pull_min_middle_gear + 
          round((params.pull_max_middle_gear - params.pull_min_middle_gear) * 
                middle_gear_01);
      
      // Calculate steering gear with offset and right scale.
      command.gear[motor_dists[2].idx] =
          params.pull_min_steer_gear +
          round((params.pull_max_steer_gear - params.pull_min_steer_gear) *
                steer_gear_01);
    }

    // Send the reaction command to motor controller module.
    buffer_motors.Push(command);
  }
}

};  // namespace bb
