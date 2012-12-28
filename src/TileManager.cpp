/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/
#include "TileManager.h"
#include <SDL_image.h>
#include <math.h>
#include "MapDownloadThread.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// tile size in pixels
static const int tdim = 256;


PointF tileForCoordinate(float lat, float lng, int zoom)
{
  float zn = static_cast<float>(1 << zoom);
  float tx = (lng + 180.0f) / 360.0f;
  float ty = (1.0f - log(tan(lat * M_PI / 180.0f) +
                         1.0f / cos(lat * M_PI / 180.0f)) / M_PI) / 2.0f;
  return PointF(tx * zn, ty * zn);
}


float longitudeFromTile(float tx, int zoom)
{
  float zn = static_cast<float>(1 << zoom);
  float lat = tx / zn * 360.0f - 180.0f;
  return lat;
}


float latitudeFromTile(float ty, int zoom)
{
  float zn = static_cast<float>(1 << zoom);
  float n = M_PI - 2 * M_PI * ty / zn;
  float lng = 180.0f / M_PI * atan(0.5f * (exp(n) - exp(-n)));
  return lng;
}


TileManager::TileManager()
: width(0), 
  height(0), 
  zoom(16), 
  texture_id(0),
  latitude(49.963734f), longitude(36.326475f) // Home
  //latitude(48.005f), longitude(11.33f) // Starnberg
  //latitude(52.52958999943302f), longitude(13.383053541183472f) // Berlin
{
  this->map_download_thread.setMapObject(this);

  m_emptyTile = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                     tdim, tdim,
                                     32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks
                                     0x000000FF, 
                                     0x0000FF00, 
                                     0x00FF0000, 
                                     0xFF000000
#else
                                     0xFF000000,
                                     0x00FF0000, 
                                     0x0000FF00, 
                                     0x000000FF
#endif
                                     );
}


TileManager::~TileManager()
{
  if(this->map_download_thread.isStarted())
    this->map_download_thread.requestShutdownAndWait();
}


void 
TileManager::invalidate()
{
  if (width <= 0 || height <= 0)
    return;

  PointF ct = tileForCoordinate(latitude, longitude, zoom);
  float tx = ct.x();
  float ty = ct.y();

  // top-left corner of the center tile
  int xp = int(width / 2 - (tx - floor(tx)) * tdim);
  int yp = int(height / 2 - (ty - floor(ty)) * tdim);

  // first tile vertical and horizontal
  int xa = (xp + tdim - 1) / tdim;
  int ya = (yp + tdim - 1) / tdim;
  int xs = static_cast<int>(tx) - xa;
  int ys = static_cast<int>(ty) - ya;

  // offset for top-left tile
  m_offset = PointI(xp - xa * tdim, yp - ya * tdim);

  // last tile vertical and horizontal
  int xe = static_cast<int>(tx) + (width - xp - 1) / tdim;
  int ye = static_cast<int>(ty) + (height - yp - 1) / tdim;

  // build a rect
  m_tilesRect.setRect(xs, ys, xe - xs + 1, ye - ys + 1);

  if(m_url.empty())
    download();
}


void 
TileManager::updateTexture(const RectI &target_rect, SDL_Surface *src)
{
  SDL_Surface *image;
  SDL_Rect area;
  Uint32 saved_flags;
  //Uint8  saved_alpha;

  RectI ir = RectI(0, 0, this->width, this->height).intersect(target_rect);

  // Convert input image (surface) to texture-compatible format
  image = SDL_CreateRGBSurface(
			       SDL_SWSURFACE,
			       ir.width(), ir.height(),
			       32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks
			       0x000000FF, 
			       0x0000FF00, 
			       0x00FF0000, 
			       0xFF000000
#else
			       0xFF000000,
			       0x00FF0000, 
			       0x0000FF00, 
			       0x000000FF
#endif
			       );
  if(image == NULL)
    return;

  /* Save the alpha blending attributes */
  saved_flags = src->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
  //saved_alpha = src->format->alpha;
  if((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
    SDL_SetAlpha(src, 0, SDL_ALPHA_TRANSPARENT);

  area.x = target_rect.x() > 0 ? 0 : -target_rect.x();
  area.y = target_rect.y() > 0 ? 0 : -target_rect.y();
  area.w = image->w;
  area.h = image->h;
  if(0 != SDL_BlitSurface(src, &area, image, NULL))
  {
    SDL_FreeSurface(image);
    return;
  }

  glBindTexture(GL_TEXTURE_2D, this->texture_id);
  // update texture
  glTexSubImage2D(GL_TEXTURE_2D, 0, 
                  ir.x(), ir.y(), 
                  image->w, image->h, 
                  GL_RGBA, GL_UNSIGNED_BYTE, 
                  image->pixels);
  glBindTexture(GL_TEXTURE_2D, 0);
  SDL_FreeSurface(image);
}


void 
TileManager::render(const RectI &rect)
{
  for(int x = 0; x <= m_tilesRect.width(); ++x)
    for(int y = 0; y <= m_tilesRect.height(); ++y)
    {
      PointI tp(x + m_tilesRect.left(), y + m_tilesRect.top());
      RectI box = this->tileRect(tp);
      if(rect.intersects(box))
      {
        SurfaceMap::const_iterator smi = m_tilePixmaps.find(tp);
        if(smi != m_tilePixmaps.end())
          this->updateTexture(box, smi->second);
        else
          this->updateTexture(box, m_emptyTile);
      }
    }
}

void 
TileManager::pan(const PointI &delta)
{
  PointF dx(delta.x() / float(tdim), delta.x() / float(tdim));
  PointF center = tileForCoordinate(latitude, longitude, zoom);
  center.x1 -= dx.x();
  center.y1 -= dx.y();
  latitude = latitudeFromTile(center.y(), zoom);
  longitude = longitudeFromTile(center.x(), zoom);
  invalidate();
}


void 
TileManager::handleNetworkData(int x, int y, const char *file_name)
{
  PointI tp(x, y);
  SDL_Surface *tile = IMG_Load(file_name);
  if(tile == NULL)
    {
      fprintf(stderr, "Can not load map tile from file %s\n", file_name);
      return;
    }
  this->m_tilePixmaps.insert(std::make_pair(tp, tile));
  this->render(RectI(0, 0, this->width, this->height));
  
  // purge unused spaces
  RectI bound = this->m_tilesRect.adjusted(-2, -2, 2, 2);
  for(SurfaceMap::iterator i = this->m_tilePixmaps.begin(); i != this->m_tilePixmaps.end(); ++i)
    if(!bound.contains(tp))
    {
      SDL_FreeSurface(i->second);
      m_tilePixmaps.erase(i);
      i = this->m_tilePixmaps.begin();
    }

  this->download();
}


void 
TileManager::setCenter(float lat, float longit)
{
  this->latitude = lat;
  this->longitude = longit;
  if(!this->map_download_thread.isStarted())
    this->map_download_thread.start();
  this->invalidate();
}


void 
TileManager::download()
{
  PointI grab(0, 0);
  for(int x = 0; x <= m_tilesRect.width() && grab.x() == 0 && grab.y() == 0; ++x)
    for(int y = 0; y <= m_tilesRect.height(); ++y)
      {
        PointI tp = m_tilesRect.topLeft() + PointI(x, y);
        if(m_tilePixmaps.find(tp) == m_tilePixmaps.end())
          {
            grab = tp;
            break;
          }
      }

  if(grab == PointI(0, 0))
    {
      m_url.clear();
      return;
    }

  this->map_download_thread.addDownloadRequest(zoom, grab.x(), grab.y());
}


RectI 
TileManager::tileRect(const PointI &tp)
{
  PointI t = tp - m_tilesRect.topLeft();
  int x = t.x() * tdim + m_offset.x();
  int y = t.y() * tdim + m_offset.y();
  return RectI(x, y, tdim, tdim);
}
