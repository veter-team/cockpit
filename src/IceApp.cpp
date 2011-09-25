/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "IceApp.h"
#include "RouterHelper.h"
#include "TxtAreaPainter.h"
#include "VideoPainter.h"
#include "SetStatusMsgCmd.h"
#include "ConnectionThread.h"
#include "GstMainThread.h"
#include "MapDownloadThread.h"
#include "TileManager.h"
#include <vehicle.h>
#include <SDL.h>
#include <IceUtil/Mutex.h>
#include <unistd.h>
#include <sstream>

#define STEERING_MOTOR_ID 0
#define ACCEL_MOTOR_ID    1
#define CAMERA_MOTOR_ID   2

// Sleep time in microseconds
static const unsigned int CMD_INTERVAL = 5 * 1000; // 5 millisecond

IceUtil::Mutex mutex;


IceApp::IceApp(TxtAreaPainter *mpainter, 
               VideoPainter *vpainter, 
               TileManager *tm,
               MapDownloadThread *mdt,
               BufferQueue *bq,
               AnimationCmdMap *cmd_map)
  : tile_manager(tm),
    map_download_thread(mdt),
    msg_painter(mpainter),
    video_painter(vpainter),
    shutdown_requested(false),
    communicator_waiting(false),
    unit_prx(0),
    buffer_queue(bq),
    anim_cmd_map(cmd_map),
    joystick_accel_prop(1), // defaults for Logitech Formula Force RX
    joystick_steering_prop(0),
    joystick_reverse_prop(7)
{
  this->map_download_thread->setMapObject(this->tile_manager);
}


IceApp::~IceApp()
{
}


void 
IceApp::printMessage(const std::string &msg)
{
  SetStatusMsgCmd *cmd = new SetStatusMsgCmd(this->msg_painter, msg);
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.code = 1;
  event.user.data1 = cmd;
  event.user.data2 = 0;
  SDL_PushEvent(&event);  
  printf("%s\n", msg.c_str());
}


void 
IceApp::requestShutdown()
{
  this->shutdown_requested = true;

  if(this->communicator_waiting)
  {
    // try to behave nice by shutdown
    this->unit_prx->cleanSensorReceiver();
    this->communicator()->shutdown();
  }

  printf("Waiting for map download thread to shutdown\n");
  this->map_download_thread->requestShutdownAndWait();
}


void 
IceApp::setControlProxies(vehicle::RemoteVehiclePrx unit)
{
  IceUtil::Mutex::Lock lock(mutex);
  this->unit_prx = unit;
}


void 
IceApp::setSensorList(const vehicleadmin::SensorList &sensor_list)
{
  std::string statusmsg = "Available sensors:";
  this->printMessage(statusmsg);
  for(vehicleadmin::SensorList::const_iterator s = sensor_list.begin();
      s != sensor_list.end(); ++s)
    {
      vehicleadmin::SensorPrx sensor = *s;
      vehicleadmin::SensorDescription descr = 
        sensor->getDescription();
      switch(descr.type)
        {
        case vehicleadmin::Camera:
          statusmsg = "Camera: ";
          this->video_painter->setStereo(false);
          break;
        case vehicleadmin::StereoCamera:
          statusmsg = "Stereo camera: ";
          this->video_painter->setStereo(true);
          break;
        case vehicleadmin::Compass:
          statusmsg = "Compass: ";
          break;
        case vehicleadmin::AccelerometerX:
          statusmsg = "Accelerometer X: ";
          break;
        case vehicleadmin::AccelerometerY:
          statusmsg = "Accelerometer Y: ";
          break;
        case vehicleadmin::AccelerometerZ:
          statusmsg = "Accelerometer Z: ";
          break;
        case vehicleadmin::GyroX:
          statusmsg = "Gyro X: ";
          break;
        case vehicleadmin::GyroY:
          statusmsg = "Gyro Y: ";
          break;
        case vehicleadmin::GyroZ:
          statusmsg = "Gyro Z: ";
          break;
        case vehicleadmin::GPS:
          this->map_download_thread->start();
          statusmsg = "GPS: ";
          break;
        case vehicleadmin::Range:
          statusmsg = "Range: ";
          break;
        case vehicleadmin::Temperature:
          statusmsg = "Temperature: ";
          break;
        case vehicleadmin::Pressure:
          statusmsg = "Pressure: ";
          break;
        case vehicleadmin::Unknown:
        default:
          statusmsg = "Unknown type: ";
          break;
        }
      statusmsg += descr.description + " (" + descr.vendorid + ")";
      this->printMessage(statusmsg);
      vehicleadmin::AdminPrx admin = sensor->getAdminInterface();
      admin->start();
    }
}


void 
IceApp::setActuatorList(const vehicleadmin::ActuatorList &actuator_list)
{
  std::string statusmsg = "Available actuators:";
  this->printMessage(statusmsg);
  for(vehicleadmin::ActuatorList::const_iterator a = actuator_list.begin();
      a != actuator_list.end(); ++a)
    {
      vehicleadmin::ActuatorPrx actuator = *a;
      vehicleadmin::ActuatorDescription descr = 
        actuator->getDescription();

      statusmsg = descr.description + "(" + descr.type + ")";
      this->printMessage(statusmsg);
                  
      vehicleadmin::AdminPrx admin = actuator->getAdminInterface();
      admin->start();
    }
}


IceUtil::ThreadControl 
IceApp::startConnectionThread(vehicle::SensorFrameReceiverPrx &video_callback)
{
  ConnectionThreadPtr t = new ConnectionThread(this, 
                                               &(this->shutdown_requested),
                                               this->unit_str_proxy,
                                               video_callback);
  IceUtil::ThreadControl tc = t->start();
  return tc;
}


int 
IceApp::run(int argc, char *argv[])
{
  std::string statusmsg;
  RouterHelperPtr rh;
  Glacier2::RouterPrx defaultRouter;
  Ice::PropertiesPtr properties = this->communicator()->getProperties();

  try
    {
      rh = new RouterHelper(this->communicator());
      defaultRouter = rh->getRouter();
    }
  catch(const Ice::Exception& ex)
    {
      std::ostringstream os;
      ex.ice_print(os);
      this->printMessage(os.str());
    }

  Ice::ObjectAdapterPtr adapter;
  try
    {
      this->printMessage("Creating object adapter");
      if(defaultRouter)
        adapter = communicator()->createObjectAdapterWithRouter("Driver", 
                                                                defaultRouter);
      else
        adapter = communicator()->createObjectAdapter("Driver");

      const char *proxyProperty = "Unit.Proxy";
      this->unit_str_proxy = properties->getProperty(proxyProperty);
      if(this->unit_str_proxy.empty())
        {
          statusmsg = "property `";
          statusmsg += proxyProperty;
          statusmsg += "' not set";
          this->printMessage(statusmsg);
          return EXIT_FAILURE;
        }

      std::string p = properties->getProperty("Joystick.accel");
      if(!p.empty())
        {
          std::istringstream is(p);
          is >> this->joystick_accel_prop;
        }
      p = properties->getProperty("Joystick.steering");
      if(!p.empty())
        {
          std::istringstream is(p);
          is >> this->joystick_steering_prop;
        }
      p = properties->getProperty("Joystick.reverse");
      if(!p.empty())
        {
          std::istringstream is(p);
          is >> this->joystick_reverse_prop;
        }
        
    }
  catch(const Ice::Exception& ex)
    {
      std::ostringstream os;
      ex.ice_print(os);
      this->printMessage(os.str());
    }

  SensorFrameReceiverIPtr sensor_callback = 
    new SensorFrameReceiverI(this->buffer_queue, 
                             this->tile_manager,
                             this->anim_cmd_map);
  Ice::ObjectPrx objprx = 
    adapter->add(sensor_callback, rh->makeId("sensorcallback"));
  if(objprx == 0)
  {
      fprintf(stderr, "Can not add sensorcallback to the object adapter\n");
      return EXIT_FAILURE;
  }
  this->sensor_callback_prx = 
    vehicle::SensorFrameReceiverPrx::uncheckedCast(objprx); 
  if(this->sensor_callback_prx == 0)
  {
      fprintf(stderr, "Can not create videocallback object\n");
      return EXIT_FAILURE;
  }

  adapter->activate();

  IceUtil::ThreadControl tc = 
    this->startConnectionThread(this->sensor_callback_prx);
  tc.join();

  GstMainThread *gst = NULL;

  const char *pipelineProperty = "Decoding.Pipeline";
  std::string decoding_pipeline = properties->getProperty(pipelineProperty);
  if(decoding_pipeline.empty())
    {
      statusmsg = "property `";
      statusmsg += pipelineProperty;
      statusmsg += "' not set. Video would not be available.";
      this->printMessage(statusmsg);
    }
  else
    gst = GstMainThread::create(argc, argv, 
                                decoding_pipeline.c_str(),
                                this->video_painter,
                                this->buffer_queue);

  if(!this->communicator_waiting && !this->shutdown_requested)
    {
      this->communicator_waiting = true;
      this->printMessage("Ready to process requests.");
      this->communicator()->waitForShutdown();
      this->communicator_waiting = false;
    }

  if(gst)
    {
      gst->requestShutdown();
      printf("Waiting for gstreamer thread to shutdown\n");
      gst->join();
      delete gst;
    }
  return EXIT_SUCCESS;
}


void 
IceApp::setSteering(short duty)
{
  if(!this->unit_prx)
    return;

  //printf("Steering duty: %i\n", duty);

  vehicle::ActuatorData d = {STEERING_MOTOR_ID, duty};
  vehicle::ActuatorFrame duties;
  duties.push_back(d);
  try
    {
      this->unit_prx->setActuators(duties);
      //usleep(CMD_INTERVAL);
    }
  catch(const Ice::Exception& ex)
    {
      this->setControlProxies(0);
      std::ostringstream os;
      ex.ice_print(os);
      this->printMessage(os.str());
      this->startConnectionThread(this->sensor_callback_prx);
    }
}


void 
IceApp::setAccel(short duty)
{
  if(!this->unit_prx)
    return;

  vehicle::ActuatorData d = {ACCEL_MOTOR_ID, duty};
  vehicle::ActuatorFrame duties;
  duties.push_back(d);
  try
    {
      this->unit_prx->setActuators(duties);
#ifdef WIN32
      Sleep(CMD_INTERVAL / 1000);
#else
      usleep(CMD_INTERVAL);
#endif
    }
  catch(const Ice::Exception& ex)
    {
      this->setControlProxies(0);
      std::ostringstream os;
      ex.ice_print(os);
      this->printMessage(os.str());
      this->startConnectionThread(this->sensor_callback_prx);
    }
}


void 
IceApp::setRotateCamera(short duty)
{
  if(!this->unit_prx)
    return;

  vehicle::ActuatorData d = {CAMERA_MOTOR_ID, duty};
  vehicle::ActuatorFrame duties;
  duties.push_back(d);
  try
    {
      this->unit_prx->setActuators(duties);
#ifdef WIN32
      Sleep(CMD_INTERVAL / 1000);
#else
      usleep(CMD_INTERVAL);
#endif
    }
  catch(const Ice::Exception& ex)
    {
      this->setControlProxies(0);
      std::ostringstream os;
      ex.ice_print(os);
      this->printMessage(os.str());
      this->startConnectionThread(this->sensor_callback_prx);
    }
}
