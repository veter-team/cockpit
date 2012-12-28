#ifndef __ACTUATORCONTROLLER_H
#define __ACTUATORCONTROLLER_H

#include <sensors.h>
#include <actuators.h>
#include <IceUtil/Handle.h>
#include <map>
#include <memory>


class ActuatorController
{
 public:
  ActuatorController(actuators::ActuatorGroupPrx actuators);
  ActuatorController(const ActuatorController &ac);
  virtual ~ActuatorController();

 public:
  virtual void processSensorData(sensors::SensorType type, 
				 const sensors::SensorData &data) = 0;

 protected:
  actuators::ActuatorGroupPrx actuators_prx;
};
typedef std::auto_ptr<ActuatorController> ActuatorControllerPtr;
typedef std::pair<short, sensors::SensorType> actctlkey_t;
typedef std::multimap<actctlkey_t, ActuatorController*> ActuatorCtlMap;
typedef std::pair<ActuatorCtlMap::iterator, ActuatorCtlMap::iterator> ActuatorCtlMapSearchRes; 


#endif // __ACTUATORCONTROLLER_H
