/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __KEYBOARDCHASSISCTL_H
#define __KEYBOARDCHASSISCTL_H

#include "ActuatorController.h"


class KeyboardChassisCtl : public ActuatorController
{
 public:
  KeyboardChassisCtl(actuators::ActuatorGroupPrx actuators);
  KeyboardChassisCtl(const KeyboardChassisCtl &kc);
  virtual ~KeyboardChassisCtl();

 public:
  virtual void processSensorData(sensors::SensorType type, 
				 const sensors::SensorData &data);

 private:
  float control_value0;
  float control_value1;
};


#endif // __KEYBOARDCHASSISCTL_H
