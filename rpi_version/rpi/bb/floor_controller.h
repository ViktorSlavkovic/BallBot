#ifndef BB_FLOOR_CONTROLLER_H_
#define BB_FLOOR_CONTROLLER_H_

#include "bb/motor_controller.h"
#include "bb/command_receiver.h"
#include "util/shared_buffer.h"

namespace bb {

class FloorController {
 public:
  static void Control(SharedBuffer<DirectionCommand>& buffer_commmands,
                      SharedBuffer<MotorControlCommand>& buffer_motors);
};

};  // namespace bb

#endif  // BB_FLOOR_CONTROLLER_H_