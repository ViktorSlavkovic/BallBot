#ifndef BB_PID_CONTROLLER_H_
#define BB_PID_CONTROLLER_H_

#include "bb/motor_controller.h"
#include "bb/mpu_reader.h"
#include "util/shared_buffer.h"

namespace bb {

class PidController {
 public:
  static void Control(SharedBuffer<MPUReader::Gravity>& buffer_sensor,
                      SharedBuffer<MotorControlCommand>& buffer_motors);
};

};  // namespace bb

#endif  // BB_PID_CONTROLLER_H_