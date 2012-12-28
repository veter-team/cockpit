#ifndef __ICEAPP_H
#define __ICEAPP_H

#include <Ice/Application.h>
#include "VisualizationManager.h"
#include "AbstractCommand.h"
#include "BufferQueue.h"
#include "VideoDecoder.h"
#include "ActuatorController.h"
#include "KeyboardSensorI.h"


class IceApp : public Ice::Application
{
 public:
  IceApp();
  virtual ~IceApp();

 public:
  virtual int run(int argc, char *argv[]);
  virtual void interruptCallback(int);

 private:
  typedef std::pair<sensors::SensorGroupPrx, sensors::SensorDescriptionSeq> sensorinfo_t;
  sensorinfo_t connectToSensor(const std::string &sensor_name) const;

  typedef std::pair<actuators::ActuatorGroupPrx, actuators::ActuatorDescriptionSeq> actuatorinfo_t;
  actuatorinfo_t connectToActuator(const std::string &actuator_name) const;

  void mainloop();
  void requestExit();
  void executeCommand(AbstractCommand *cmd) const;
  void handleKeyEvent(SDL_KeyboardEvent *key);

 private:
  ActuatorCtlMap actuator_map;
  VideoDecoder video_decoder;
  BufferQueue buffer_queue;
  VisualizationManager visuals;
  bool should_stop;
  KeyboardSensorIPtr keyboard_sensor;
  admin::StatePrx chassis_state_prx;
};


#endif // __ICEAPP_H
