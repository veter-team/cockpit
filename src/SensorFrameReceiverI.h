/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __SENSORFRAMERECEIVERI_H
#define __SENSORFRAMERECEIVERI_H

#include <sensors.h>
#include <IceUtil/Handle.h>
#include "ActuatorController.h"
#include "AnimationCmd.h"


class BufferQueue;
class TileManager;


class SensorFrameReceiverI : public sensors::SensorFrameReceiver
{
 public:
  SensorFrameReceiverI(sensors::SensorType type,
		       float data_min,
		       float data_max,
		       BufferQueue *bq, 
                       TileManager *tm,
                       AnimationCmdMap *cmd_map,
		       ActuatorCtlMap *actuator_map);

 public:
  virtual void nextSensorFrame(const sensors::SensorFrame &frame, 
                               const Ice::Current& = Ice::Current());

 private:
  const sensors::SensorType sensor_type;
  float data_min_val;
  float data_max_val;
  BufferQueue *buffer_queue;
  TileManager *tile_manager;
  AnimationCmdMap *anim_cmd_map;
  ActuatorCtlMap *sensor_actuator_map;
};

typedef IceUtil::Handle<SensorFrameReceiverI> SensorFrameReceiverIPtr;


#endif // __SENSORFRAMERECEIVERI_H
