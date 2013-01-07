/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __VIDEOPAINTER_H
#define __VIDEOPAINTER_H

#include <SDL_opengl.h>
#include <SDL_thread.h>
#include <vector>


class TxtAreaPainter;


class VideoPainter
{
 public:
  VideoPainter(TxtAreaPainter *painter, 
               size_t width = 640, 
               size_t height = 480);
  virtual ~VideoPainter();

 public:
  void setTextureId(GLuint id);
  void receiveBuffer(unsigned char *data, size_t size);
  void paint() const;

  void setStereo(bool is_stereo);

 protected:
  virtual void processImageBuffer(unsigned char *data, size_t size);
  void setupTexture();

  TxtAreaPainter *msgpainter;
  size_t framewidth, frameheight;
  SDL_mutex *mut;
  unsigned char *texdata;
  bool paint_stereo;
  GLuint videotex;
};

typedef std::vector<VideoPainter*> VideoPainterList;


class EdgeVideoPainter : public VideoPainter
{
 public:
  EdgeVideoPainter(TxtAreaPainter *painter, 
		   size_t width = 640, 
		   size_t height = 480);

 protected:
  virtual void processImageBuffer(unsigned char *data, size_t size);
};

#endif // __VIDEOPAINTER_H
