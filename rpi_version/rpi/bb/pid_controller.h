#ifndef BB_PID_CONTROLLER_H_
#define BB_PID_CONTROLLER_H_

#include "bb/motor_controller.h"
#include "bb/mpu_reader.h"
#include "util/shared_buffer.h"
#include "bb/command_receiver.h"

namespace bb {

class PidController {
public:
    static void Control(SharedBuffer<MPUReader::Gravity>& buffer_sensor,
                        SharedBuffer<MotorControlCommand>& buffer_motors);
    // static void Control(SharedBuffer<std::string>& buffer_udp_sender,
    //                     SharedBuffer<bb::DirectionCommand>& buffer_direction);
};

};  // namespace bb

#endif  // BB_PID_CONTROLLER_H_