#ifndef __KEYBOARDSENSOR_H
#define __KEYBOARDSENSOR_H

#include <IceUtil/Handle.h>
#include <sensors.h>
#include <SDL_events.h>


class KeyboardSensorI : public sensors::SensorGroup
{
 public:
  KeyboardSensorI(admin::StatePrx &prx);
  ~KeyboardSensorI();

 public:
  virtual admin::StatePrx getStateInterface(const Ice::Current& = Ice::Current());

  virtual sensors::SensorDescriptionSeq getSensorDescription(const Ice::Current& = Ice::Current());

  virtual sensors::SensorFrame getCurrentValues(const Ice::Current& = Ice::Current());

  virtual bool setSensorReceiver(const sensors::SensorFrameReceiverPrx& callback,
				 const Ice::Current& = ::Ice::Current());

  virtual void cleanSensorReceiver(const Ice::Current& = ::Ice::Current());

  void setRunning(bool running);
  void handleKeyEvent(SDL_KeyboardEvent *key);

 private:
  admin::StatePrx state_prx;
  sensors::SensorFrameReceiverPrx sensor_cb;
  bool is_running;
  SDL_KeyboardEvent last_event;
};

typedef IceUtil::Handle<KeyboardSensorI> KeyboardSensorIPtr;

#endif // __KEYBOARDSENSOR_H
