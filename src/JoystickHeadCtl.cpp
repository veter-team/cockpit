#include "JoystickHeadCtl.h"
#include <strings.h>
#include <sstream>
#include <Ice/Application.h>
#include <math.h>


JoystickHeadCtl::JoystickHeadCtl(actuators::ActuatorGroupPrx acts)
  : ActuatorController(acts),
    prev_pos(10.0f)
{
}


JoystickHeadCtl::JoystickHeadCtl(const JoystickHeadCtl &jh)
  : ActuatorController(jh),
    prev_pos(jh.prev_pos)
{
}


JoystickHeadCtl::~JoystickHeadCtl()
{
}


// Implementation of the signum function
template <typename T> int sign(T val) 
{
    return (T(0) < val) - (val < T(0));
}


void 
JoystickHeadCtl::processSensorData(sensors::SensorType type, 
				   const sensors::SensorData &data)
{
  // Sanity check
  if(!this->actuators_prx 
     || type != sensors::Joystick 
     || data.floatdata.empty())
    return;
  
  Ice::LoggerPtr log = Ice::Application::communicator()->getLogger();

  actuators::ActuatorData actuator_cmd;
  actuators::ActuatorFrame actuator_frame;

  // For the very first time, we need to set the servo at the known
  // position. For this reason we first move servo to the left-most
  // position and then move to the middle. 10.0f is "impossible" value
  // which indicates that we are entering this function for the first
  // time.
  if(this->prev_pos == 10.0f)
    {
      this->prev_pos = 0.5f;
      actuator_cmd.id = 0;
      actuator_cmd.speed = -1.0f;
      actuator_cmd.distance = 1.0f;
      actuator_frame.push_back(actuator_cmd);
      // Position the servo to the left-most position;
      try 
	{
	  this->actuators_prx->setActuatorsNoWait(actuator_frame);
	}
      catch(const Ice::Exception &ex)
	{
	  std::ostringstream os;
	  os << "Error sending commands to actuator.\n";
	  ex.ice_print(os);
	  log->warning(os.str());
	  return;
	}
      actuator_frame.clear();
      actuator_cmd.id = 0;
      actuator_cmd.speed = 1.0f;
      actuator_cmd.distance = 0.5f;
      actuator_frame.push_back(actuator_cmd);
      // Position the servo to the middle
      try 
	{
	  this->actuators_prx->setActuatorsNoWait(actuator_frame);
	}
      catch(const Ice::Exception &ex)
	{
	  std::ostringstream os;
	  os << "Error sending commands to actuator.\n";
	  ex.ice_print(os);
	  log->warning(os.str());
	  return;
	}
    }

  // Scale to 0:1 interval.
  // Use [3] for knob and [2] for stick rotation axis
  float new_pos = (data.floatdata[3] + 1.0f) / 2.0f;

  actuator_frame.clear();

  actuator_cmd.id = 0;
  // Uncomment if axis 3 (knob) is used
  actuator_cmd.speed = sign(new_pos - this->prev_pos);
  // Uncomment if axis 2 (stick rotation) is used
  //actuator_cmd.speed = sign(this->prev_pos - new_pos);
  
  actuator_cmd.distance = fabs(new_pos - this->prev_pos);
  // Add some toolerance to avoid sending almost the 
  // same position again and again
  if(actuator_cmd.distance  < 0.01f)
    return;
  this->prev_pos = new_pos;
  actuator_frame.push_back(actuator_cmd);

  // Send commands to actuators
  try 
    {
      this->actuators_prx->setActuatorsNoWait(actuator_frame);
    }
  catch(const Ice::Exception &ex)
    {
      std::ostringstream os;
      os << "Error sending commands to actuator.\n";
      ex.ice_print(os);
      log->warning(os.str());
    }
}
