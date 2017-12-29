#include "bb/floor_controller.h"

namespace bb {

void FloorController::Control(
  SharedBuffer<DirectionCommand>& buffer_commmands,
  SharedBuffer<MotorControlCommand>& buffer_motors) {
    MotorControlCommand command;
    
      while (true) {
    
        auto cmd = buffer_commmands.Pop();
        
        switch (cmd) {
          case FORWARD: {
            command.gear[0] = 0;
            command.gear[1] = 255;
            command.gear[2] = 255;
            command.dir[1] = 0;
            command.dir[2] = 1;
            break;
          }
          case ROTATE_LEFT: {
            command.gear[0] = 200;
            command.gear[1] = 200;
            command.gear[2] = 200;
            command.dir[0] = 0;
            command.dir[1] = 0;
            command.dir[2] = 0;
            break;
          }
          case ROTATE_RIGHT: {
            command.gear[0] = 200;
            command.gear[1] = 200;
            command.gear[2] = 200;
            command.dir[0] = 1;
            command.dir[1] = 1;
            command.dir[2] = 1;
            break;
          }
          case STOP: {
            command.gear[0] = 0;
            command.gear[1] = 0;
            command.gear[2] = 0;
            command.dir[0] = 1;
            command.dir[1] = 1;
            command.dir[2] = 1;
            break;
          }
        }
        buffer_motors.Push(command);
        
      }
}

};  // namespace bb