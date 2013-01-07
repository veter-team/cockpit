#include "IceApp.h"
#include <sstream>
#include "RouterHelper.h"
#include "SensorFrameReceiverI.h"
#include "KeyboardAdminI.h"
#include "JoystickChassisCtl.h"
#include "JoystickHeadCtl.h"
#include "KeyboardChassisCtl.h"
#include "KeyboardHeadCtl.h"

using namespace std;


IceApp::IceApp()
  : should_stop(false)
{
  this->callbackOnInterrupt();
}


IceApp::~IceApp()
{
  // Be nice and stop chassis motors on exit
  if(this->chassis_state_prx)
      this->chassis_state_prx->stop();
}


void 
IceApp::interruptCallback(int i)
{

  Ice::Application::interruptCallback(i);
}


void 
IceApp::requestExit()
{
  this->should_stop = true;
  this->video_decoder.requestShutdown();
}


int 
IceApp::run(int argc, char *argv[])
{
  RouterHelperPtr rh;
  Glacier2::RouterPrx defaultRouter;
  Ice::PropertiesPtr props = this->communicator()->getProperties();
  Ice::LoggerPtr log = this->communicator()->getLogger();

  // Try to create router with firewall support using Glacier service
  try
    {
      rh = new RouterHelper(this->communicator());
      defaultRouter = rh->getRouter();
    }
  catch(const Ice::Exception& ex)
    {
      ostringstream os;
      ex.ice_print(os);
      log->warning(os.str());
    }

  // Create object adapter for sensor callback and keyboard sensor servants
  Ice::ObjectAdapterPtr adapter;
  if(defaultRouter)
    adapter = communicator()->createObjectAdapterWithRouter("Cockpit", 
							    defaultRouter);
  else
    adapter = communicator()->createObjectAdapter("Cockpit");

  if(this->visuals.init(log, props))
    {
      log->error("Visuals initialization failed");
      return EXIT_FAILURE;
    }

  Ice::ObjectPrx objprx;

  // Initialize keyboard sensor (and admin) servants
  KeyboardAdminIPtr keyboard_admin = new KeyboardAdminI();
  objprx = adapter->add(keyboard_admin, 
			this->communicator()->stringToIdentity("keyboard-sensor-admin"));
  admin::StatePrx keyboard_admin_prx = 
    admin::StatePrx::uncheckedCast(objprx); 

  this->keyboard_sensor = new KeyboardSensorI(keyboard_admin_prx);
  keyboard_admin->setSensorGroup(this->keyboard_sensor);
  objprx = 
    adapter->add(this->keyboard_sensor, 
		 this->communicator()->stringToIdentity("keyboard-sensor"));

  adapter->activate();

  admin::StatePrx state_prx;

  // Initialize actuators list

  // Our chassis has two motors
  actuatorinfo_t cactinfo = this->connectToActuator("Chassis");
  actuators::ActuatorGroupPrx cactuator_prx = cactinfo.first;
  //actuators::ActuatorDescriptionSeq cactdescr = actinfo.second;

  // Servo motor to rotate the head with cameras and front sonar
  actuatorinfo_t hactinfo = this->connectToActuator("Head");
  actuators::ActuatorGroupPrx hactuator_prx = hactinfo.first;
  //actuators::ActuatorDescriptionSeq hactdescr = actinfo.second;

  // Initialize joystick callback servant
  sensorinfo_t info = this->connectToSensor("Joystick");
  sensors::SensorGroupPrx sensor_prx = info.first;
  sensors::SensorDescriptionSeq descr = info.second;
  ActuatorControllerPtr joy_chassis_ctl;
  ActuatorControllerPtr joy_head_ctl;
  if(sensor_prx && !descr.empty())
    {
      actctlkey_t key = std::make_pair(descr[0].id, descr[0].type);
      if(cactuator_prx)
	{
	  joy_chassis_ctl.reset(new JoystickChassisCtl(cactuator_prx));
	  this->actuator_map.insert(std::make_pair(key, 
						   joy_chassis_ctl.get()));
	  this->chassis_state_prx = cactuator_prx->getStateInterface();
	  this->chassis_state_prx->start();
	}
      if(hactuator_prx)
	{
	  joy_head_ctl.reset(new JoystickHeadCtl(hactuator_prx));
	  this->actuator_map.insert(std::make_pair(key, 
						   joy_head_ctl.get()));
	  state_prx = hactuator_prx->getStateInterface();
	  state_prx->start();
	}
      SensorFrameReceiverIPtr joystick_callback = 
	new SensorFrameReceiverI(descr[0].type,
				 descr[0].minvalue,
				 descr[0].maxvalue,
				 &this->buffer_queue, 
				 &this->visuals.tile_manager,
				 &this->visuals.sensor_animations_map,
				 &this->actuator_map);
      objprx = 
	adapter->add(joystick_callback, rh->makeId("joystickcallback"));
      sensors::SensorFrameReceiverPrx joystick_callback_prx = 
	sensors::SensorFrameReceiverPrx::uncheckedCast(objprx); 
      sensor_prx->setSensorReceiver(joystick_callback_prx);
      state_prx = sensor_prx->getStateInterface();
      state_prx->start();
    }

  // Initialize keyboard callback servant
  info = this->connectToSensor("Keyboard");
  sensor_prx = info.first;
  descr = info.second;
  ActuatorControllerPtr kbd_chassis_ctl;
  ActuatorControllerPtr kbd_head_ctl;
  if(sensor_prx && !descr.empty())
    {
      actctlkey_t key = std::make_pair(descr[0].id, descr[0].type);
      if(cactuator_prx)
	{
	  kbd_chassis_ctl.reset(new KeyboardChassisCtl(cactuator_prx));
	  this->actuator_map.insert(std::make_pair(key, 
						   kbd_chassis_ctl.get()));
	  state_prx = cactuator_prx->getStateInterface();
	  state_prx->start();
	}
      if(hactuator_prx)
	{
	  kbd_head_ctl.reset(new KeyboardHeadCtl(hactuator_prx));
	  this->actuator_map.insert(std::make_pair(key, 
						   kbd_head_ctl.get()));
	  state_prx = hactuator_prx->getStateInterface();
	  state_prx->start();
	}

      SensorFrameReceiverIPtr kbd_callback = 
	new SensorFrameReceiverI(descr[0].type,
				 descr[0].minvalue,
				 descr[0].maxvalue,
				 &this->buffer_queue, 
				 &this->visuals.tile_manager,
				 &this->visuals.sensor_animations_map,
				 &this->actuator_map);
      objprx = 
	adapter->add(kbd_callback, 
		     this->communicator()->stringToIdentity("kbdcallback"));
      sensors::SensorFrameReceiverPrx kbd_callback_prx = 
	sensors::SensorFrameReceiverPrx::uncheckedCast(objprx); 
      sensor_prx->setSensorReceiver(kbd_callback_prx);
      state_prx = sensor_prx->getStateInterface();
      state_prx->start();
    }

  // Initialize sonars callback servant
  info = this->connectToSensor("Sonars");
  sensor_prx = info.first;
  descr = info.second;
  if(sensor_prx && !descr.empty())
    {
      SensorFrameReceiverIPtr sonars_callback = 
	new SensorFrameReceiverI(descr[0].type,
				 descr[0].minvalue,
				 descr[0].maxvalue,
				 &this->buffer_queue, 
				 &this->visuals.tile_manager,
				 &this->visuals.sensor_animations_map,
				 NULL);
      objprx = 
	adapter->add(sonars_callback, rh->makeId("sonarscallback"));
      sensors::SensorFrameReceiverPrx sonars_callback_prx = 
	sensors::SensorFrameReceiverPrx::uncheckedCast(objprx); 
      sensor_prx->setSensorReceiver(sonars_callback_prx);
      admin::StatePrx state_prx = sensor_prx->getStateInterface();
      state_prx->start();
    }

  // Initialize compass callback servant
  info = this->connectToSensor("Compass");
  sensor_prx = info.first;
  descr = info.second;
  if(sensor_prx && !descr.empty())
    {
      SensorFrameReceiverIPtr compass_callback = 
	new SensorFrameReceiverI(descr[0].type,
				 descr[0].minvalue,
				 descr[0].maxvalue,
				 &this->buffer_queue, 
				 &this->visuals.tile_manager,
				 &this->visuals.sensor_animations_map,
				 NULL);
      objprx = 
	adapter->add(compass_callback, rh->makeId("compasscallback"));
      sensors::SensorFrameReceiverPrx compass_callback_prx = 
	sensors::SensorFrameReceiverPrx::uncheckedCast(objprx); 
      sensor_prx->setSensorReceiver(compass_callback_prx);
      admin::StatePrx state_prx = sensor_prx->getStateInterface();
      state_prx->start();
    }

  // Initialize camera callback servant
  info = this->connectToSensor("Camera");
  sensor_prx = info.first;
  descr = info.second;
  if(sensor_prx && !descr.empty())
    {
      const std::string prop_name = "Decoding.Pipeline";
      std::string dec_pipeline = props->getProperty(prop_name);
      if(dec_pipeline.empty())
	{
	  log->warning(prop_name + " property not set. No video.");
	}
      else
	{
	  this->video_decoder.initAndStart(argc, argv, 
					   dec_pipeline.c_str(),
					   this->visuals.video_painters,
					   &this->buffer_queue);
				       
	  SensorFrameReceiverIPtr camera_callback = 
	    new SensorFrameReceiverI(descr[0].type,
				     descr[0].minvalue,
				     descr[0].maxvalue,
				     &this->buffer_queue, 
				     &this->visuals.tile_manager,
				     &this->visuals.sensor_animations_map,
				     NULL);
	  objprx = 
	    adapter->add(camera_callback, rh->makeId("cameracallback"));
	  sensors::SensorFrameReceiverPrx camera_callback_prx = 
	    sensors::SensorFrameReceiverPrx::uncheckedCast(objprx); 
	  sensor_prx->setSensorReceiver(camera_callback_prx);
	  state_prx = sensor_prx->getStateInterface();
	  state_prx->start();
	}
    }
  
  this->mainloop();

  // Be nice and stop chassis motors on exit
  if(this->chassis_state_prx)
    {
      this->chassis_state_prx->stop();
      this->chassis_state_prx = 0;
    }

  return EXIT_SUCCESS;
}

// TODO: mrege connectToSensor() and connectToActuator() in one
// template function.

IceApp::sensorinfo_t 
IceApp::connectToSensor(const std::string &sensor_name) const
{
  Ice::PropertiesPtr props = this->communicator()->getProperties();
  Ice::LoggerPtr log = this->communicator()->getLogger();

  try
    {
      log->print(string("Connecting to ") + sensor_name + " sensor");
      const string prop_name = sensor_name + ".proxy";
      string sensor_str_proxy = props->getProperty(prop_name);
      sensors::SensorGroupPrx sensor_prx;
      if(sensor_str_proxy.empty())
	log->warning(prop_name 
		     + string(" property is not set. No ")
		     + sensor_name + ".");
      else
	{
	  Ice::ObjectPrx objprx = 
	    this->communicator()->stringToProxy(sensor_str_proxy);
	  sensor_prx = sensors::SensorGroupPrx::checkedCast(objprx);
	  if(!sensor_prx)
	    log->warning(string("Can not connect to ")
			 + sensor_name 
			 + string(" sensor. No ")
			 + sensor_name + ".");
	  else
	    {
	      string msg = "Connected to sensor group ";
	      log->print(msg + sensor_name);
	      sensors::SensorDescriptionSeq descr = 
		sensor_prx->getSensorDescription();
	      for(sensors::SensorDescriptionSeq::const_iterator d = descr.begin();
		  d != descr.end(); ++d)
		{
		  ostringstream os;
		  os << "  Vendor id: " << d->vendorid << endl;
		  os << "  Description: " << d->description << endl;
		  os << "  Id: " << d->id << endl;
		  os << "  Min value: " << d->minvalue << endl;
		  os << "  Max value: " << d->maxvalue << endl;
		  os << "  Recommended refresh rate, Hz: " << d->refreshrate;
		  log->print(os.str());
		}
	      return make_pair(sensor_prx, descr);
	    }
	}
    }
  catch(const Ice::Exception& ex)
    {
      ostringstream os;
      os << "Can not connect to " << sensor_name << " sensor. No "
	 << sensor_name + ".\n";
      ex.ice_print(os);
      log->warning(os.str());
    }

  return make_pair(sensors::SensorGroupPrx(), 
		   sensors::SensorDescriptionSeq());
}


IceApp::actuatorinfo_t 
IceApp::connectToActuator(const std::string &actuator_name) const
{
  Ice::PropertiesPtr props = this->communicator()->getProperties();
  Ice::LoggerPtr log = this->communicator()->getLogger();

  try
    {
      log->print(string("Connecting to ") + actuator_name + " actuators");
      const string prop_name = actuator_name + ".proxy";
      string actuator_str_proxy = props->getProperty(prop_name);
      actuators::ActuatorGroupPrx actuator_prx;
      if(actuator_str_proxy.empty())
	log->warning(prop_name 
		     + string(" property is not set. No ")
		     + actuator_name + ".");
      else
	{
	  Ice::ObjectPrx objprx = 
	    this->communicator()->stringToProxy(actuator_str_proxy);
	  actuator_prx = 
	    actuators::ActuatorGroupPrx::checkedCast(objprx);
	  if(!actuator_prx)
	    log->warning(string("Can not connect to ")
			 + actuator_name 
			 + string(" actuator. No ")
			 + actuator_name + ".");
	  else
	    {
	      string msg = "Connected to actuator group ";
	      log->print(msg + actuator_name);
	      actuators::ActuatorDescriptionSeq descr = 
		actuator_prx->getActuatorDescription();
	      for(actuators::ActuatorDescriptionSeq::const_iterator d = descr.begin();
		  d != descr.end(); ++d)
		{
		  ostringstream os;
		  os << "  Vendor id: " << d->vendorid << endl;
		  os << "  Description: " << d->description << endl;
		  os << "  Id: " << d->id << endl;
		  log->print(os.str());
		}
	      return make_pair(actuator_prx, descr);
	    }
	}
    }
  catch(const Ice::Exception &ex)
    {
      std::ostringstream os;
      os << "Can not connect to actuator " + actuator_name + ".\n";
      ex.ice_print(os);
      log->warning(os.str());
    }

  return make_pair(actuators::ActuatorGroupPrx(), 
		   actuators::ActuatorDescriptionSeq());
}


void 
IceApp::mainloop()
{
  // used to collect events
  SDL_Event event;

  this->visuals.drawScene();

  // wait for events
  while(!this->should_stop)
    {
      // handle the events in the queue 
      while(SDL_WaitEvent(&event))
        {
          switch(event.type)
            {
            case SDL_VIDEORESIZE:
              // handle resize event 
              this->visuals.resizeWindow(event.resize.w, event.resize.h);
              break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
              // handle key press and release
              this->handleKeyEvent(&event.key);
              break;

            case SDL_USEREVENT:
              if(event.user.code && event.user.data1)
                this->executeCommand((AbstractCommand*)event.user.data1);
              break;

            case SDL_QUIT:
              /* handle quit requests */
              this->requestExit();
              break;

            default:
              this->visuals.drawScene();
	    }

          if(!this->should_stop)
            this->visuals.drawScene();
          else
            break;
        }
    }

  // clean ourselves up and exit
  SDL_Quit();
}


void 
IceApp::executeCommand(AbstractCommand *cmd) const
{
  cmd->execute();
  if(cmd->shouldDelete())
    delete cmd;
}


void 
IceApp::handleKeyEvent(SDL_KeyboardEvent *key)
{
  this->keyboard_sensor->handleKeyEvent(key);
}
