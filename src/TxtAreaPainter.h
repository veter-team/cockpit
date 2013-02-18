/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __TXTAREAPAINTER_H
#define __TXTAREAPAINTER_H

#include <string>
#include <list>

class TxtAreaPainter
{
 public:
  TxtAreaPainter();

 public:
  void addString(const std::string &msg);

 protected:
  typedef std::list<std::string> StringList;
  StringList strings;
};


#endif // __TXTAREAPAINTER_H
