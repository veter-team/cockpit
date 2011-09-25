/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __ANIMATIONCMD_H
#define __ANIMATIONCMD_H

#include "AbstractCommand.h"
#include "AnimationController.h"
#include <map>


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
typedef std::map<short, AnimationCmd> AnimationCmdMap;

#endif // __ANIMATIONCMD_H
