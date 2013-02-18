/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "SetStatusMsgCmd.h"
#include "TxtAreaPainter.h"


SetStatusMsgCmd::SetStatusMsgCmd(TxtAreaPainter *p, const std::string &msg)
  : painter(p),
    message(msg)
{
}


void 
SetStatusMsgCmd::execute()
{
  if(painter)
    painter->addString(this->message);
}
