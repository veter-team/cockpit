/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "KeyboardAdminI.h"


KeyboardAdminI::KeyboardAdminI()
{
}


void 
KeyboardAdminI::setSensorGroup(KeyboardSensorIPtr &sg)
{
  this->sensor_group = sg;
}


void
KeyboardAdminI::start(const Ice::Current&)
{
  this->sensor_group->setRunning(true);
}


void
KeyboardAdminI::stop(const Ice::Current&)
{
  this->sensor_group->setRunning(false);
}
