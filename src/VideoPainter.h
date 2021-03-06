/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
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
  virtual bool processImageBuffer(unsigned char *data, size_t size);
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
  virtual bool processImageBuffer(unsigned char *data, size_t size);

  size_t frame_counter;
};

#endif // __VIDEOPAINTER_H
