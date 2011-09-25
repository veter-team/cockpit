/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __ICEAPP_H
#define __ICEAPP_H

#include <Ice/Application.h>
#include <IceUtil/Monitor.h>
#include <vehicle.h>
#include "SensorFrameReceiverI.h"
#include "AnimationCmd.h"

class TxtAreaPainter;
class VideoPainter;
class BufferQueue;
class TileManager;
class MapDownloadThread;


class IceApp : public Ice::Application
{
 public:
  IceApp(TxtAreaPainter *mpainter, 
         VideoPainter *vpainter, 
         TileManager *tm,
         MapDownloadThread *mdt,
         BufferQueue *bq,
         AnimationCmdMap *cmd_map);
  virtual ~IceApp();

 public:
  virtual int run(int argc, char *argv[]);
  void requestShutdown();
  void printMessage(const std::string &msg);

  void setSteering(short duty);
  void setAccel(short duty);
  void setRotateCamera(short duty);

  void setControlProxies(vehicle::RemoteVehiclePrx unit);
  void setSensorList(const vehicleadmin::SensorList &sensor_list);
  void setActuatorList(const vehicleadmin::ActuatorList &actuator_list);

  const char *getDecodingPipeline();

  short getJoystickAccelAxis() const {return this->joystick_accel_prop;}
  short getJoystickSteeringAxis() const {return this->joystick_steering_prop;}
  short getJoystickReverseButton() const {return this->joystick_reverse_prop;}
  
 private:
  IceUtil::ThreadControl startConnectionThread(vehicle::SensorFrameReceiverPrx &video_callback);

  TileManager *tile_manager;
  MapDownloadThread *map_download_thread;

  std::string unit_str_proxy;
  TxtAreaPainter *msg_painter;
  VideoPainter *video_painter;
  bool shutdown_requested;
  bool communicator_waiting;
  vehicle::RemoteVehiclePrx unit_prx;
  BufferQueue *buffer_queue;
  AnimationCmdMap *anim_cmd_map;
  vehicle::SensorFrameReceiverPrx sensor_callback_prx;
  short joystick_accel_prop;
  short joystick_steering_prop;
  short joystick_reverse_prop;
};


#endif // __ICEAPP_H
