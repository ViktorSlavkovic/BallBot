#ifndef BB_PID_CONTROLLER_H_
#define BB_PID_CONTROLLER_H_

#include "bb/motor_controller.h"
#include "bb/mpu_reader.h"
#include "util/shared_buffer.h"
#include "bb/command_receiver.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

namespace bb {

using std::endl;
using std::istream;
using std::ostream;
using std::ostringstream;
using std::istringstream;
using std::string;

constexpr const double kPi = 3.14159265358979323846; 

// TODO: Rename, this is not PID.
class PidController {
public:
    struct ParameterSet {
     public:

      ParameterSet() {}

      explicit ParameterSet(string hash_string) {
        istringstream sin(hash_string);
        sin >> *this;
      }

      // [0, Pi/3], 0 - pull only, Pi/3 - push only
      double push_angle_threshold = kPi / 180 * 30;
      
      // do we steer at all or just disable the stepper?
      bool steer = true;

      // [0, 255]
      uint8_t push_min_steer_gear = 150;
      // [0, 255] >= min_steer_gear
      uint8_t push_max_steer_gear = 255;
      // [0, 255]
      uint8_t push_min_middle_gear = 200;
      // [0, 255] >= min_middle_gear
      uint8_t push_max_middle_gear = 230;
      // [0, 255]
      uint8_t push_min_lowest_gear = 230;
      // [0, 255] >= min_worst_gear
      uint8_t push_max_lowest_gear = 255;

      // [0, 255]
      uint8_t pull_min_steer_gear = 150;
      // [0, 255] >= min_steer_gear
      uint8_t pull_max_steer_gear = 255;
      // [0, 255]
      uint8_t pull_min_middle_gear = 230;
      // [0, 255] >= min_middle_gear
      uint8_t pull_max_middle_gear = 255;
      // [0, 255]
      uint8_t pull_min_highest_gear = 200;
      // [0, 255] >= min_worst_gear
      uint8_t pull_max_highest_gear = 230;
      
      // 0, 1
      double push_alpha_importance_lowest = 0.8;
      double push_product_importance_lowest = 0.25;
      double push_lin_importance_lowest = 0.3;
      
      double push_alpha_importance_middle = 0.8;
      double push_product_importance_middle = 0.25;
      double push_lin_importance_middle = 0.3;
      
      double push_alpha_importance_steer = 0.8;
      double push_product_importance_steer = 0.25;
      double push_lin_importance_steer = 0.3;

      double pull_alpha_importance_highest = 0.8;
      double pull_product_importance_highest = 0.25;
      double pull_lin_importance_highest = 0.3;
      
      double pull_alpha_importance_middle = 0.8;
      double pull_product_importance_middle = 0.25;
      double pull_lin_importance_middle = 0.3;
      
      double pull_alpha_importance_steer = 0.8;
      double pull_product_importance_steer = 0.25;
      double pull_lin_importance_steer = 0.3;

      string ToString() const {
        char buffer[4096];
        sprintf(buffer,
          "Params:                      \n"
          "  push_angle_threshold: %7.2f\n"
          "  steer:                %7.d\n"
          "                            \n"
          "  push_min_steer_gear:  %7.d\n"
          "  push_max_steer_gear:  %7.d\n"
          "  push_min_middle_gear: %7.d\n"
          "  push_max_middle_gear: %7.d\n"
          "  push_min_lowest_gear: %7.d\n"
          "  push_max_lowest_gear: %7.d\n"
          "  pull_min_steer_gear:  %7.d\n"
          "  pull_max_steer_gear:  %7.d\n"
          "  pull_min_middle_gear: %7.d\n"
          "  pull_max_middle_gear: %7.d\n"
          "  pull_min_highest_gear:%7.d\n"
          "  pull_max_highest_gear:%7.d\n"
          "                            \n"
          "  push_alpha_importance_lowest:    %7.2f\n"
          "  push_product_importance_lowest:  %7.2f\n"
          "  push_lin_importance_lowest:      %7.2f\n"
          "  push_alpha_importance_middle:    %7.2f\n"
          "  push_product_importance_middle:  %7.2f\n"
          "  push_lin_importance_middle:      %7.2f\n"
          "  push_alpha_importance_steer:     %7.2f\n"
          "  push_product_importance_steer:   %7.2f\n"
          "  push_lin_importance_steer:       %7.2f\n"
          "  pull_alpha_importance_highest:   %7.2f\n"
          "  pull_product_importance_highest: %7.2f\n"
          "  pull_lin_importance_highest:     %7.2f\n"
          "  pull_alpha_importance_middle:    %7.2f\n"
          "  pull_product_importance_middle:  %7.2f\n"
          "  pull_lin_importance_middle:      %7.2f\n"
          "  pull_alpha_importance_steer:     %7.2f\n"
          "  pull_product_importance_steer:   %7.2f\n"
          "  pull_lin_importance_steer:       %7.2f\n",
          push_angle_threshold,
          steer,
          push_min_steer_gear,
          push_max_steer_gear,
          push_min_middle_gear,
          push_max_middle_gear,
          push_min_lowest_gear,
          push_max_lowest_gear,
          pull_min_steer_gear,
          pull_max_steer_gear,
          pull_min_middle_gear,
          pull_max_middle_gear,
          pull_min_highest_gear,
          pull_max_highest_gear,
          push_alpha_importance_lowest,
          push_product_importance_lowest,
          push_lin_importance_lowest,
          push_alpha_importance_middle,
          push_product_importance_middle,
          push_lin_importance_middle,
          push_alpha_importance_steer,
          push_product_importance_steer,
          push_lin_importance_steer,
          pull_alpha_importance_highest,
          pull_product_importance_highest,
          pull_lin_importance_highest,
          pull_alpha_importance_middle,
          pull_product_importance_middle,
          pull_lin_importance_middle,
          pull_alpha_importance_steer,
          pull_product_importance_steer,
          pull_lin_importance_steer
        );
        return string(buffer);
      }

      string ToHashString() const {
        ostringstream sout;
        sout << *this;
        return sout.str();
      }

      operator string() const {
        return ToHashString();
      }

      // Storing to stream (used for writing to file)
      friend ostream& operator<< (ostream& out, const ParameterSet& ps) {
        out.precision(2);
        out << ps.push_angle_threshold << endl;
        out << ps.steer << endl;
        out << ps.push_min_steer_gear << endl;
        out << ps.push_max_steer_gear << endl;
        out << ps.push_min_middle_gear << endl;
        out << ps.push_max_middle_gear << endl;
        out << ps.push_min_lowest_gear << endl;
        out << ps.push_max_lowest_gear << endl;
        out << ps.pull_min_steer_gear << endl;
        out << ps.pull_max_steer_gear << endl;
        out << ps.pull_min_middle_gear << endl;
        out << ps.pull_max_middle_gear << endl;
        out << ps.pull_min_highest_gear << endl;
        out << ps.pull_max_highest_gear << endl;
        out << ps.push_alpha_importance_lowest << endl;
        out << ps.push_product_importance_lowest << endl;
        out << ps.push_lin_importance_lowest << endl;
        out << ps.push_alpha_importance_middle << endl;
        out << ps.push_product_importance_middle << endl;
        out << ps.push_lin_importance_middle << endl;
        out << ps.push_alpha_importance_steer << endl;
        out << ps.push_product_importance_steer << endl;
        out << ps.push_lin_importance_steer << endl;
        out << ps.pull_alpha_importance_highest << endl;
        out << ps.pull_product_importance_highest << endl;
        out << ps.pull_lin_importance_highest << endl;
        out << ps.pull_alpha_importance_middle << endl;
        out << ps.pull_product_importance_middle << endl;
        out << ps.pull_lin_importance_middle << endl;
        out << ps.pull_alpha_importance_steer << endl;
        out << ps.pull_product_importance_steer << endl;
        out << ps.pull_lin_importance_steer << endl;
        return out;
      }

      // Reading from stream (used for reading from file).
      friend istream& operator>> (istream& in, ParameterSet& ps) {
        in >> ps.push_angle_threshold;
        in >> ps.steer;
        in >> ps.push_min_steer_gear;
        in >> ps.push_max_steer_gear;
        in >> ps.push_min_middle_gear;
        in >> ps.push_max_middle_gear;
        in >> ps.push_min_lowest_gear;
        in >> ps.push_max_lowest_gear;
        in >> ps.pull_min_steer_gear;
        in >> ps.pull_max_steer_gear;
        in >> ps.pull_min_middle_gear;
        in >> ps.pull_max_middle_gear;
        in >> ps.pull_min_highest_gear;
        in >> ps.pull_max_highest_gear;
        in >> ps.push_alpha_importance_lowest;
        in >> ps.push_product_importance_lowest;
        in >> ps.push_lin_importance_lowest;
        in >> ps.push_alpha_importance_middle;
        in >> ps.push_product_importance_middle;
        in >> ps.push_lin_importance_middle;
        in >> ps.push_alpha_importance_steer;
        in >> ps.push_product_importance_steer;
        in >> ps.push_lin_importance_steer;
        in >> ps.pull_alpha_importance_highest;
        in >> ps.pull_product_importance_highest;
        in >> ps.pull_lin_importance_highest;
        in >> ps.pull_alpha_importance_middle;
        in >> ps.pull_product_importance_middle;
        in >> ps.pull_lin_importance_middle;
        in >> ps.pull_alpha_importance_steer;
        in >> ps.pull_product_importance_steer;
        in >> ps.pull_lin_importance_steer;
        return in;
      }

      static ParameterSet Generate() {
        ParameterSet p;
        p.steer = (rand() % 2) ? true : false;

        uint8_t x;
        x = rand() % 256;
        p.push_max_middle_gear = p.push_min_lowest_gear = x;
        p.push_min_middle_gear = rand() % (x + 1);
        p.push_max_lowest_gear = 255; //x + (rand() % (256 - x + 1));
        x = rand() % 256;
        p.pull_min_middle_gear = p.pull_max_highest_gear = x;
        p.pull_max_middle_gear = 255; //x + (rand() % (256 - x + 1));
        p.pull_min_highest_gear = rand() % (x + 1);
        x = rand() % 256;
        p.push_max_steer_gear = x;
        p.push_min_steer_gear = rand() % (x + 1);
        x = rand() % 256;
        p.pull_max_steer_gear = x;
        p.pull_min_steer_gear = rand() % (x + 1);
        
        p.push_alpha_importance_lowest = (rand() % 101) * 0.01;
        p.push_product_importance_lowest = (rand() % 101) * 0.01;
        p.push_lin_importance_lowest = (rand() % 101) * 0.01;
        
        p.push_alpha_importance_middle = (rand() % 101) * 0.01;
        p.push_product_importance_middle = (rand() % 101) * 0.01;
        p.push_lin_importance_middle = (rand() % 101) * 0.01;
        
        p.push_alpha_importance_steer = (rand() % 101) * 0.01;
        p.push_product_importance_steer = (rand() % 101) * 0.01;
        p.push_lin_importance_steer = (rand() % 101) * 0.01;
  
        p.pull_alpha_importance_highest = (rand() % 101) * 0.01;
        p.pull_product_importance_highest = (rand() % 101) * 0.01;
        p.pull_lin_importance_highest = (rand() % 101) * 0.01;
        
        p.pull_alpha_importance_middle = (rand() % 101) * 0.01;
        p.pull_product_importance_middle = (rand() % 101) * 0.01;
        p.pull_lin_importance_middle = (rand() % 101) * 0.01;
        
        p.pull_alpha_importance_steer = (rand() % 101) * 0.01;
        p.pull_product_importance_steer = (rand() % 101) * 0.01;
        p.pull_lin_importance_steer =  (rand() % 101) * 0.01;

        return p;
      }
    };

    static void Control(SharedBuffer<MPUReader::Gravity>& buffer_sensor,
        SharedBuffer<MotorControlCommand>& buffer_motors,
        const ParameterSet& params
    );
};

};  // namespace bb

#endif  // BB_PID_CONTROLLER_H_