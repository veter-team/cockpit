#ifndef __KEYBOARDADMINI_H
#define __KEYBOARDADMINI_H

#include <IceUtil/Handle.h>
#include <admin.h>
#include "KeyboardSensorI.h"


class KeyboardAdminI : public admin::State
{
 public:
  KeyboardAdminI();

  void setSensorGroup(KeyboardSensorIPtr &sg);

 public:
  virtual void start(const Ice::Current& = ::Ice::Current());
  virtual void stop(const Ice::Current& = ::Ice::Current());

 private:
  KeyboardSensorIPtr sensor_group;
};

typedef IceUtil::Handle<KeyboardAdminI> KeyboardAdminIPtr;

#endif // __KEYBOARDADMINI_H
