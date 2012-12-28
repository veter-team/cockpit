#include "ActuatorController.h"
#include <Ice/Application.h>


ActuatorController::ActuatorController(actuators::ActuatorGroupPrx actuators)
  : actuators_prx(actuators)
{
}


ActuatorController::ActuatorController(const ActuatorController &ac)
  : actuators_prx(ac.actuators_prx)
{
}


ActuatorController::~ActuatorController()
{
}
