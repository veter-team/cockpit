/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __VIDEOPAINTER_H
#define __VIDEOPAINTER_H

#include <SDL_opengl.h>
#include <SDL_thread.h>

class TxtAreaPainter;


class VideoPainter
{
 public:
  VideoPainter(TxtAreaPainter *painter, 
               size_t width = 640, 
               size_t height = 480);
  ~VideoPainter();

 public:
  void setTextureId(GLuint id);
  void receiveBuffer(unsigned char *data, size_t size);
  void paint() const;

  void setStereo(bool is_stereo);

 private:
  void setupTexture();

  TxtAreaPainter *msgpainter;
  size_t framewidth, frameheight;
  SDL_mutex *mut;
  unsigned char *texdata;
  bool paint_stereo;
  GLuint videotex;
};


#endif // __VIDEOPAINTER_H
