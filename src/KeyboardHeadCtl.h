/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __KEYBOARDHEADCTL_H
#define __KEYBOARDHEADCTL_H

#include "ActuatorController.h"


class KeyboardHeadCtl : public ActuatorController
{
 public:
  KeyboardHeadCtl(actuators::ActuatorGroupPrx actuators);
  KeyboardHeadCtl(const KeyboardHeadCtl &jc);
  virtual ~KeyboardHeadCtl();

 public:
  virtual void processSensorData(sensors::SensorType type, 
				 const sensors::SensorData &data);
 private:
  float prev_pos;
};


#endif // __KEYBOARDHEADCTL_H
