#include "KeyboardChassisCtl.h"
#include <strings.h>
#include <sstream>
#include <Ice/Application.h>
#include <math.h>
#include <SDL_events.h>

#define STEERING_INC 0.1f
#define ACCEL_INC 0.1f


KeyboardChassisCtl::KeyboardChassisCtl(actuators::ActuatorGroupPrx acts)
  : ActuatorController(acts),
    control_value0(0.0f),
    control_value1(0.0f)
{
}


KeyboardChassisCtl::KeyboardChassisCtl(const KeyboardChassisCtl &kc)
  : ActuatorController(kc),
    control_value0(kc.control_value0),
    control_value1(kc.control_value1)
{
}


KeyboardChassisCtl::~KeyboardChassisCtl()
{
}


void 
KeyboardChassisCtl::processSensorData(sensors::SensorType type, 
				      const sensors::SensorData &data)
{
  // Sanity check
  if(!this->actuators_prx 
     || type != sensors::Keyboard 
     || data.bytedata.empty()
     || data.shortdata.size() < 2)
    return;

  const Uint8 event_type = data.bytedata.front();

  if(event_type == SDL_KEYDOWN)
    {
      const SDLKey sym = SDLKey(data.shortdata.front());
      //const SDLMod mod = SDLMod(*(data.shortdata.begin() + 1));
      switch(sym)
	{
	case SDLK_LEFT:
	  if(control_value0 >= (-1.0 + STEERING_INC))
	      control_value0 -= STEERING_INC;
	  break;

	case SDLK_RIGHT:
	  if(control_value0 <= (1.0 - STEERING_INC))
	    control_value0 += STEERING_INC;
	  break;
		      
	case SDLK_UP:
	  if(control_value1 >= (-1.0 + ACCEL_INC))
	    control_value1 -= ACCEL_INC;
	  break;

	case SDLK_DOWN:
	  if(control_value1 <= (1.0 - ACCEL_INC))
	    control_value1 += ACCEL_INC;
	  break;
		      
	default:
	  return;
	}
    }

  actuators::ActuatorFrame actuator_frame;
  actuators::ActuatorData actuator_cmd;

  // Initialize actuator control sttucture
  // Forward/backward - axis 1, left/right - axis 0.
  // Actuator 0 is the left wheel, 1 - right

  // If both axis are less then sensibility, then do nothing
  float sensibility = 0.10; // 10%

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
