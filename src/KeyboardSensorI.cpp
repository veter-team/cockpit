#include "KeyboardSensorI.h"

//2100ms, 5Hz
#define UPDATE_INTERVAL (200 * 1000)


KeyboardSensorI::KeyboardSensorI(admin::StatePrx &prx)
  : state_prx(prx), is_running(false)
{
}


KeyboardSensorI::~KeyboardSensorI()
{
}


admin::StatePrx 
KeyboardSensorI::getStateInterface(const Ice::Current&)
{
  return this->state_prx;
}


sensors::SensorDescriptionSeq 
KeyboardSensorI::getSensorDescription(const Ice::Current&)
{
  static const sensors::SensorDescription descr = 
    {
      // sensor unique (for the group) id
      0, sensors::Keyboard,
      // data range
      SDLK_FIRST, SDLK_LAST,
      // recommended sensor refresh rate
      1000000 / UPDATE_INTERVAL,
      "Generic keyboard sensor", "Keyboard"
    };

  return sensors::SensorDescriptionSeq(1, descr);
}


void 
buildSensorFrame(SDL_KeyboardEvent *key,
		 sensors::SensorFrame &frame)
{
  sensors::SensorData d;
  d.sensorid = 0;
  d.bytedata.push_back(key->type);
  d.shortdata.push_back(key->keysym.sym);
  d.shortdata.push_back(key->keysym.mod);
  frame.push_back(d);
}


sensors::SensorFrame 
KeyboardSensorI::getCurrentValues(const Ice::Current&)
{
  sensors::SensorFrame frame;
  buildSensorFrame(&this->last_event, frame);
  return frame;
}


bool 
KeyboardSensorI::setSensorReceiver(const sensors::SensorFrameReceiverPrx& callback, 
				   const Ice::Current&)
{
  this->sensor_cb = sensors::SensorFrameReceiverPrx::uncheckedCast(callback->ice_oneway()->ice_timeout(2000));;

  return true;
}


void 
KeyboardSensorI::cleanSensorReceiver(const Ice::Current&)
{
  this->sensor_cb = 0;
}


void 
KeyboardSensorI::setRunning(bool running)
{
  this->is_running = running;
}


void 
KeyboardSensorI::handleKeyEvent(SDL_KeyboardEvent *key)
{
  this->last_event = *key;
  try
    {
      if(this->is_running && this->sensor_cb)
	{
	  sensors::SensorFrame frame;
	  buildSensorFrame(key, frame);
	  if(!frame.empty())
	    this->sensor_cb->nextSensorFrame(frame);
	}
    }
  catch(const Ice::Exception& ex)
    {
      //cout << ex << '\n';
      //cout << "Forgetting sensor callback receiver\n";
      this->sensor_cb = 0;
      this->is_running = false;
    }
}
