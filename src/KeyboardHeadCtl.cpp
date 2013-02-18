/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "KeyboardHeadCtl.h"
#include <strings.h>
#include <sstream>
#include <Ice/Application.h>
#include <math.h>
#include <SDL_events.h>

#define SERVO_INC 0.05f


KeyboardHeadCtl::KeyboardHeadCtl(actuators::ActuatorGroupPrx acts)
  : ActuatorController(acts),
    prev_pos(10.0f)
{
}


KeyboardHeadCtl::KeyboardHeadCtl(const KeyboardHeadCtl &jh)
  : ActuatorController(jh),
    prev_pos(jh.prev_pos)
{
}


KeyboardHeadCtl::~KeyboardHeadCtl()
{
}


// Implementation of the signum function
template <typename T> int sign(T val) 
{
    return (T(0) < val) - (val < T(0));
}


void 
KeyboardHeadCtl::processSensorData(sensors::SensorType type, 
				   const sensors::SensorData &data)
{
  // Sanity check
  if(!this->actuators_prx 
     || type != sensors::Keyboard 
     || data.bytedata.empty()
     || data.shortdata.size() < 2)
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
      actuator_frame.clear();
    }

  float new_pos = this->prev_pos;
  const Uint8 event_type = data.bytedata.front();

  if(event_type == SDL_KEYDOWN)
    {
      const SDLKey sym = SDLKey(data.shortdata.front());
      //const SDLMod mod = SDLMod(*(data.shortdata.begin() + 1));
      switch(sym)
	{
	case SDLK_a:
	  if(new_pos >= (0.0f + SERVO_INC))
	      new_pos -= SERVO_INC;
	  break;

	case SDLK_d:
	  if(new_pos <= (1.0f - SERVO_INC))
	    new_pos += SERVO_INC;
	  break;
		      
	default:
	  return;
	}
    }

  actuator_cmd.id = 0;
  // Uncomment if axis 3 (knob) is used
  //actuator_cmd.speed = sign(new_pos - this->prev_pos);
  // Uncomment if axis 2 (stick rotation) is used
  actuator_cmd.speed = sign(this->prev_pos - new_pos);
  
  actuator_cmd.distance = SERVO_INC;
  // Avoid sending the same position again and again
  if(new_pos == this->prev_pos)
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
