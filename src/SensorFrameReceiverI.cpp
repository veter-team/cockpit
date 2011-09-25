/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "SensorFrameReceiverI.h"
#include "BufferQueue.h"
#include "RangeMap.h"
#include "TileManager.h"
#include <unistd.h>
#include <SDL.h>


SensorFrameReceiverI::SensorFrameReceiverI(BufferQueue *bq,
                                           TileManager *tm,
                                           AnimationCmdMap *cmd_map)
  : buffer_queue(bq),
    tile_manager(tm),
    anim_cmd_map(cmd_map)
{
}

void 
SensorFrameReceiverI::nextSensorFrame(const vehicle::SensorFrame &frame, 
                                      const Ice::Current &current)
{
  AnimationCmd *cmd;
  AnimationCmdMap::iterator ac;
  for(vehicle::SensorFrame::const_iterator data = frame.begin();
      data != frame.end(); ++data)
    {
      ac = this->anim_cmd_map->find(data->sensorid);
      if(ac == this->anim_cmd_map->end())
        cmd = NULL;
      else
        cmd = &(ac->second);

      switch(data->sensorid)
        {
        case 0: // Camera
          if(!data->bytedata.empty())
            this->buffer_queue->store(data->bytedata);
          break;

        case 2: // Sonar
          if(cmd && !data->intdata.empty())
            cmd->setPercentage(range_map(0, 80, 
                                         0, 100, 
                                         data->intdata[0]));
          break;

        case 3: // Compass
          if(cmd && !data->intdata.empty())
            cmd->setPercentage(range_map(0, 255,//3599, 
                                         0, 100, 
                                         data->intdata[0]));
          break;

        case 4: // GPS
          if(data->floatdata.size() > 1)
            {
              static float old_lat = 0;
              static float old_long = 0;
              if(data->floatdata[0] != old_lat
                 || data->floatdata[1] != old_long)
                {
                  old_lat = data->floatdata[0];
                  old_long = data->floatdata[1];
                  this->tile_manager->setCenter(data->floatdata[0], 
                                                data->floatdata[1]);
                }
            }
          break;

        default:
          cmd = NULL;
          break;
        }
      if(cmd)
        {
          // Trigger repaint
          SDL_Event event;
          event.type = SDL_USEREVENT;
          event.user.code = 1;
          event.user.data1 = cmd;
          event.user.data2 = 0;
          SDL_PushEvent(&event);  
        }
    }
}
