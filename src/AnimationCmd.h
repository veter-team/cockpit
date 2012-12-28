#ifndef __ANIMATIONCMD_H
#define __ANIMATIONCMD_H

#include <map>
#include <sensors.h>
#include "AbstractCommand.h"
#include "AnimationController.h"


class AnimationCmd : public AbstractCommand
{
 public:
  AnimationCmd(AnimationControllerList *al);

 public:
  virtual void execute();
  virtual bool shouldDelete() {return false;}
  void setPercentage(short perc) {this->percentage = perc;}

 private:
  AnimationControllerList *anim_ctl_list;
  short percentage;
};

// Maps sensor id to the corresponding animation command
struct sensorid_t
{
  short id;
  sensors::SensorType type;
  size_t data_idx;

  sensorid_t() : id(0), type(sensors::Unknown), data_idx(0) {}

  sensorid_t(short i, sensors::SensorType t, size_t d) : id(i), type(t), data_idx(d) {}

  sensorid_t(const sensorid_t &rhs) : id(rhs.id), type(rhs.type), data_idx(rhs.data_idx) {}

  sensorid_t & operator = (const sensorid_t &rhs) 
  { id = rhs.id; type = rhs.type; data_idx = rhs.data_idx; return *this; }

  bool operator == (const sensorid_t &rhs) const 
  { return id == rhs.id && type == rhs.type && data_idx == rhs.data_idx; }

  bool operator < (const sensorid_t &rhs) const 
  { 
    if(id < rhs.id)
      return true;
    else if(id > rhs.id)
      return false;
    else if(type < rhs.type)
      return true;
    else if(type > rhs.type)
      return false;
    else if(data_idx < rhs.data_idx)
      return true;

    return false;
  }

};
typedef std::map<sensorid_t, AnimationCmd> AnimationCmdMap;

#endif // __ANIMATIONCMD_H
