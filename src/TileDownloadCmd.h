/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __TILEDOWNLOADCMD_H
#define __TILEDOWNLOADCMD_H

#include <string>
#include "AbstractCommand.h"
#include "TileManager.h"


class TileDownloadCmd : public AbstractCommand
{
 public:
  TileDownloadCmd(TileManager *m) : map(m) {}

 public:
  virtual void execute() {map->handleNetworkData(x, y, file_name.c_str());}

 public:
  int x;
  int y;
  std::string file_name;

 private:
   TileManager *map;
};

#endif // __TILEDOWNLOADCMD_H
