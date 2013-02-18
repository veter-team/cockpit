/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "JoystickChassisCtl.h"
#include <strings.h>
#include <sstream>
#include <Ice/Application.h>
#include <math.h>

#define SMOOTH_WND_SIZE 3


JoystickChassisCtl::JoystickChassisCtl(actuators::ActuatorGroupPrx acts)
  : ActuatorController(acts), 
    smooth_window0(SMOOTH_WND_SIZE, 0.0f),
    smooth_window1(SMOOTH_WND_SIZE, 0.0f)
{
}


JoystickChassisCtl::JoystickChassisCtl(const JoystickChassisCtl &jc)
  : ActuatorController(jc),
    smooth_window0(jc.smooth_window0),
    smooth_window1(jc.smooth_window1)
{
}


JoystickChassisCtl::~JoystickChassisCtl()
{
}


void 
JoystickChassisCtl::processSensorData(sensors::SensorType type, 
				      const sensors::SensorData &data)
{
  // Sanity check
  if(!this->actuators_prx 
     || type != sensors::Joystick 
     || data.floatdata.empty())
    return;

  const float control_value0 = data.floatdata[0];
  const float control_value1 = data.floatdata[1];

  // If both axis are less then sensibility, then do nothing
  const float sensibility = 0.20; // 20%

  actuators::ActuatorFrame actuator_frame;
  actuators::ActuatorData actuator_cmd;

  // Initialize actuator control sttucture
  // Forward/backward - axis 1, left/right - axis 0.
  // Actuator 0 is the left wheel, 1 - right

  // If acceleration is less than sensibility, then use
  // "on-place" rotation mode where wheels will rotate in the
  // opposite directions
  if(fabs(control_value1) <= sensibility)
    {
      if(fabs(control_value0) > sensibility)
	{// On-place rotation mode
	  actuator_cmd.id = 0;
	  actuator_cmd.speed = control_value0 * 100.0;
	  actuator_cmd.distance = 100;
	  actuator_frame.push_back(actuator_cmd);

	  actuator_cmd.id = 1;
	  actuator_cmd.speed = -control_value0 * 100.0;
	  actuator_cmd.distance = 100;
	  actuator_frame.push_back(actuator_cmd);
	}
      else
	{
	  actuator_cmd.id = 0;
	  actuator_cmd.speed = 0;
	  actuator_cmd.distance = 0;
	  actuator_frame.push_back(actuator_cmd);

	  actuator_cmd.id = 1;
	  actuator_cmd.speed = 0;
	  actuator_cmd.distance = 0;
	  actuator_frame.push_back(actuator_cmd);
	}
    }
  else
    {// Normal cruise mode
      actuator_cmd.id = 0;
      actuator_cmd.speed = -control_value1 * 100.0;
      actuator_cmd.distance = 100;
      if(control_value0 < 0) 
	{
	  // Left turn. Slow down the left wheel depending on the
	  // value of the left/right axis
	  actuator_cmd.speed = 
	    actuator_cmd.speed - actuator_cmd.speed * (-control_value0);
	}
      actuator_frame.push_back(actuator_cmd);

      actuator_cmd.id = 1;
      actuator_cmd.speed = -control_value1 * 100.0;
      actuator_cmd.distance = 100;
      if(control_value0 > 0)
	{
	  // Right turn. Slow down the right wheel depending on the
	  // value of the left/right axis
	  actuator_cmd.speed = 
	    actuator_cmd.speed - actuator_cmd.speed * control_value0;
	}
      actuator_frame.push_back(actuator_cmd);
    }

  // Send commands to actuators
  try 
    {
      this->actuators_prx->setActuatorsNoWait(actuator_frame);
    }
  catch(const Ice::Exception &ex)
    {
      Ice::LoggerPtr log = Ice::Application::communicator()->getLogger();
      std::ostringstream os;
      os << "Error sending commands to actuator.\n";
      ex.ice_print(os);
      log->warning(os.str());
    }
}
