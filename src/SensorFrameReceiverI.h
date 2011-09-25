/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __SENSORFRAMERECEIVERI_H
#define __SENSORFRAMERECEIVERI_H

#include <vehicle.h>
#include <IceUtil/Handle.h>
#include "AnimationCmd.h"


class BufferQueue;
class TileManager;

class SensorFrameReceiverI : public vehicle::SensorFrameReceiver
{
 public:
  SensorFrameReceiverI(BufferQueue *bq, 
                       TileManager *tm,
                       AnimationCmdMap *cmd_map);

 public:
  virtual void nextSensorFrame(const vehicle::SensorFrame &frame, 
                               const Ice::Current& = Ice::Current());

 private:
   BufferQueue *buffer_queue;
   TileManager *tile_manager;
   AnimationCmdMap *anim_cmd_map;
};

typedef IceUtil::Handle<SensorFrameReceiverI> SensorFrameReceiverIPtr;


#endif // __SENSORFRAMERECEIVERI_H
