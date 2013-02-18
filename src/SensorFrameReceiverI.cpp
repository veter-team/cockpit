/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "SensorFrameReceiverI.h"
#include "BufferQueue.h"
#include "RangeMap.h"
#include "TileManager.h"
#include <unistd.h>
#include <list>
#include <SDL.h>


SensorFrameReceiverI::SensorFrameReceiverI(sensors::SensorType type,
					   float data_min,
					   float data_max,
					   BufferQueue *bq,
                                           TileManager *tm,
                                           AnimationCmdMap *cmd_map,
					   ActuatorCtlMap *actuator_map)
  : sensor_type(type),
    data_min_val(data_min),
    data_max_val(data_max),
    buffer_queue(bq),
    tile_manager(tm),
    anim_cmd_map(cmd_map),
    sensor_actuator_map(actuator_map)
{
}

void 
SensorFrameReceiverI::nextSensorFrame(const sensors::SensorFrame &frame, 
                                      const Ice::Current &current)
{
  typedef std::list<AnimationCmd*> AnimCmdList;
  AnimCmdList cmds;
  sensorid_t sensor_key;
  AnimationCmdMap::iterator animcmd_iter;
  ActuatorCtlMapSearchRes ctl_search_res;
  for(sensors::SensorFrame::const_iterator data = frame.begin();
      data != frame.end(); ++data)
    {
      sensor_key.id = data->sensorid;
      sensor_key.type = this->sensor_type;
      sensor_key.data_idx = 0;
      animcmd_iter = this->anim_cmd_map->find(sensor_key);
      if(animcmd_iter != this->anim_cmd_map->end())
        cmds.push_back(&(animcmd_iter->second));

      switch(this->sensor_type)
        {
        case sensors::Camera:
        case sensors::StereoCamera:
          if(!data->bytedata.empty())
            this->buffer_queue->store(data->bytedata);
	  break;

        case sensors::Joystick:
          if(!data->floatdata.empty())
	    {
	      // First visualize sensor data

	      // Joysticks are sending axis values in the same
	      // floatdata array. Here we will iterate through known
	      // animation commands and see if there are commands for
	      // each particular axis.
	      cmds.clear();
	      Ice::FloatSeq::size_type i = 0;
	      for(Ice::FloatSeq::const_iterator v = data->floatdata.begin();
		  v != data->floatdata.end(); ++v, ++i)
		{
		  sensor_key.data_idx = i;
		  animcmd_iter = this->anim_cmd_map->find(sensor_key);
		  if(animcmd_iter != this->anim_cmd_map->end())
		    {
		      AnimationCmd *cmd = &animcmd_iter->second;
		      cmd->setPercentage(range_map(this->data_min_val, 
						   this->data_max_val, 
						   0.0, 100.0, 
						   *v));
		      cmds.push_back(cmd);
		    }
		}
	    }
	  break;

        case sensors::Keyboard:
          if(!data->bytedata.empty() && !data->shortdata.size() < 2)
	    {
	      static const float steering_inc = 0.1;
	      static const float accel_inc = 0.1;
	      static float steering_perc = 0;
	      static float accel_perc = 0;
	      Uint8 event_type = data->bytedata.front();
	      if(event_type == SDL_KEYDOWN)
		{
		  cmds.clear();
		  const SDLKey sym = SDLKey(data->shortdata.front());
		  //const SDLMod mod = SDLMod(*(data->shortdata.begin() + 1));
		  for(size_t i = 0; i < 2; ++i)
		    {
		      sensor_key.data_idx = i;
		      animcmd_iter = this->anim_cmd_map->find(sensor_key);
		      if(animcmd_iter != this->anim_cmd_map->end())
			{
			  AnimationCmd *cmd = &animcmd_iter->second;

			  switch(sym)
			    {
			    case SDLK_LEFT:
			      if(i == 0 && steering_perc >= (-1.0f + steering_inc))
				{
				  steering_perc -= steering_inc;
				  cmd->setPercentage(range_map(-1.0f, 1.0f,
							       0.0f, 100.0f,
							       steering_perc));
				  cmds.push_back(cmd);
				}
			      break;

			    case SDLK_RIGHT:
			      if(i == 0 
				 && steering_perc <= (1.0f - steering_inc))
				{
				  steering_perc += steering_inc;
				  cmd->setPercentage(range_map(-1.0f, 1.0f,
							       0.0f, 100.0f,
							       steering_perc));
				  cmds.push_back(cmd);
				}
			      break;
		      
			    case SDLK_DOWN:
			      if(i == 1 && accel_perc >= (-1.0f - accel_inc))
				{
				  accel_perc -= accel_inc;
				  cmd->setPercentage(range_map(-1.0f, 1.0f,
							       0.0f, 100.0f,
							       accel_perc));
				  cmds.push_back(cmd);
				}
			      break;

			    case SDLK_UP:
			      if(i == 1 && accel_perc <= (1.0f - accel_inc))
				{
				  accel_perc += accel_inc;
				  cmd->setPercentage(range_map(-1.0f, 1.0f,
							       0.0f, 100.0f,
							       accel_perc));
				  cmds.push_back(cmd);
				}
			      break;
		      
			    default:
			      break;
			    }
			}
		    }
		}
	    }
	  break;

        case sensors::Compass:
          if(!cmds.empty() && !data->floatdata.empty())
            cmds.front()->setPercentage(range_map(this->data_min_val, 
						   this->data_max_val, 
						   0.0, 100.0, 
						   data->floatdata[0]));
	  break;

	case sensors::Range:
	  if(!cmds.empty() && !data->shortdata.empty())
	    cmds.front()->setPercentage(range_map(this->data_min_val, 
						   this->data_max_val, 
						   0.0, 100.0, 
						   float(data->shortdata[0])));
	  break;

	case sensors::GPS: // lattitude, longitude, altitude
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
	  break;
	}
      for(AnimCmdList::const_iterator c = cmds.begin(); c != cmds.end(); ++c)
	{
	  // Trigger repaint
	  SDL_Event event;
	  event.type = SDL_USEREVENT;
	  event.user.code = 1;
	  event.user.data1 = *c;
	  event.user.data2 = 0;
	  SDL_PushEvent(&event);  
	}
      cmds.clear();

      // Now pass sensor data to actuators if requested
      if(this->sensor_actuator_map)
	{
	  ActuatorCtlMap::key_type k 
	    = std::make_pair(sensor_key.id, sensor_key.type);
	  ctl_search_res = this->sensor_actuator_map->equal_range(k);
	  for(ActuatorCtlMap::iterator controller = ctl_search_res.first;
	      controller != ctl_search_res.second; ++controller)
	    {
	      controller->second->processSensorData(sensor_key.type, 
						    *data);
	    }
	}

    }
}
