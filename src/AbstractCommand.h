/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __ABSTRACTCOMMAND_H
#define __ABSTRACTCOMMAND_H


class AbstractCommand
{
 public:
  AbstractCommand() {}
  virtual ~AbstractCommand() {}

 public:
  virtual bool shouldDelete() {return true;}

  virtual void execute() = 0;
};


#endif //__ABSTRACTCOMMAND_H
