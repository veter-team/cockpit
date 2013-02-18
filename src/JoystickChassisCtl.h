/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __JOYSTICKCHASSISCTL_H
#define __JOYSTICKCHASSISCTL_H

#include "ActuatorController.h"
#include <list>


class JoystickChassisCtl : public ActuatorController
{
 public:
  JoystickChassisCtl(actuators::ActuatorGroupPrx actuators);
  JoystickChassisCtl(const JoystickChassisCtl &jc);
  virtual ~JoystickChassisCtl();

 public:
  virtual void processSensorData(sensors::SensorType type, 
				 const sensors::SensorData &data);
 private:
  typedef std::list<float> FloatArray;
  FloatArray smooth_window0;
  FloatArray smooth_window1;
};


#endif // __JOYSTICKCHASSISCTL_H
