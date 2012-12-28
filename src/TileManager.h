/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/
#ifndef __TILEMANAGER_H
#define __TILEMANAGER_H

#include <SDL.h>
#include <SDL_opengl.h>
#include <map>
#include <string>
#include "rect.h"
#include "MapDownloadThread.h"


typedef PointT<int> PointI;
typedef PointT<float> PointF;
typedef Rect<int> RectI;


class TileManager
{
public:
  TileManager();
  ~TileManager();

public:
  void invalidate();
  void render(const RectI &rect);
  void pan(const PointI &delta);
  void handleNetworkData(int x, int y, const char *file_name);
  void setCenter(float lat, float longit);

public:
  int width;
  int height;
  int zoom;
  GLuint texture_id;

protected:
  RectI tileRect(const PointI &tp);

private:
  void download();
  void updateTexture(const RectI &target_rect, SDL_Surface *src);

private:
  float latitude;
  float longitude;
  MapDownloadThread map_download_thread;
  PointI m_offset;
  RectI m_tilesRect;
  SDL_Surface *m_emptyTile;
  typedef std::map<PointI, SDL_Surface*> SurfaceMap;
  SurfaceMap m_tilePixmaps;
  std::string m_url;
};


#endif // __TILEMANAGER_H
