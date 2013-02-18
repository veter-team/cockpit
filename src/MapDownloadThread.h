/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __MAPDOWNLOADTHREAD_H
#define __MAPDOWNLOADTHREAD_H

#include <curl/curl.h>
#include <map>
#include <string>
#include "SDLThread.h"
#include <SDL_mutex.h>
#include "rect.h"

class TileManager;

class MapDownloadThread : public SDLThread
{
public:
  MapDownloadThread();
  virtual ~MapDownloadThread();

public:
  void setMapObject(TileManager *m);
  void addDownloadRequest(int zoom, int x, int y);
  virtual int run();
  virtual int requestShutdownAndWait();

private:
  void signalDownloadComplete(int x, int y, const std::string &file_name) const;

  struct ActiveDownloadInfo
  {
    int x;
    int y;
    std::string file_name;
    FILE *ostream;
  };
  typedef std::map<CURL*, ActiveDownloadInfo> ActiveDownloadMap;

  ActiveDownloadMap active_downloads;
  SDL_mutex *mut;
  SDL_cond *cond;
  CURLM *multiple_handle;
  bool new_handle;
  int running_handles;
  TileManager *map;
};


#endif // __MAPDOWNLOADTHREAD_H
