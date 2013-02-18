/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __JOYSTICKHEADCTL_H
#define __JOYSTICKHEADCTL_H

#include "ActuatorController.h"


class JoystickHeadCtl : public ActuatorController
{
 public:
  JoystickHeadCtl(actuators::ActuatorGroupPrx actuators);
  JoystickHeadCtl(const JoystickHeadCtl &jc);
  virtual ~JoystickHeadCtl();

 public:
  virtual void processSensorData(sensors::SensorType type, 
				 const sensors::SensorData &data);
 private:
  float prev_pos;
};


#endif // __JOYSTICKHEADCTL_H
