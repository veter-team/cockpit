/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "AnimationCmd.h"


AnimationCmd::AnimationCmd(AnimationControllerList *al)
  : anim_ctl_list(al),
    percentage(0)
{
}


void 
AnimationCmd::execute()
{
  AnimateList(*(this->anim_ctl_list), this->percentage);
}
