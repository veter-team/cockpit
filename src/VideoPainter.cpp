/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "VideoPainter.h"
#include "PrintStatusMsg.h"
#include "toanaglyph.h"
#include "toedge.h"
#include <stdio.h>
#include <SDL.h>


VideoPainter::VideoPainter(TxtAreaPainter *painter,
                           size_t width, 
                           size_t height)
  : msgpainter(painter),
    framewidth(width),
    frameheight(height),
    paint_stereo(false),
    videotex(0)
{
  this->mut = SDL_CreateMutex();
  this->texdata = new unsigned char[this->framewidth * this->frameheight * 4];
  //memset(this->texdata, 0x80808080, this->framewidth * this->frameheight * 4);
}


VideoPainter::~VideoPainter()
{
  if(this->mut)
    SDL_DestroyMutex(mut);

  if(this->texdata)
    delete texdata;
}


void 
VideoPainter::setTextureId(GLuint id)
{
  this->videotex = id;
}


void 
VideoPainter::setStereo(bool is_stereo)
{
  this->paint_stereo = is_stereo;
}


void 
VideoPainter::receiveBuffer(unsigned char *data, size_t size)
{
  if(SDL_mutexP(this->mut) == -1)
    {
      printStatusMessage("Could not lock texture mutex", this->msgpainter);
      return;
    }

  if(this->videotex)
    {
      // Make a copy of the frame
      if(size != this->framewidth * this->frameheight * 4)
        fprintf(stderr, "strange buffer size received from gst\n");

      if(this->processImageBuffer(data, size))
	{
	  // Trigger repaint
	  SDL_Event event;
	  event.type = SDL_USEREVENT;
	  event.user.code = 0;
	  event.user.data1 = 0;
	  event.user.data2 = 0;
	  SDL_PushEvent(&event);
	}
    }

  if(SDL_mutexV(this->mut) == -1)
    printStatusMessage("Could not unlock texture mutex", this->msgpainter);
}


bool 
VideoPainter::processImageBuffer(unsigned char *data, size_t size)
{
  if(this->paint_stereo)
    {
      IplImage *image = frameToAnaglyph(data, this->framewidth, this->frameheight);
      memcpy(this->texdata, image->imageData, size);
      cvReleaseImage(&image);
    }
  else
    memcpy(this->texdata, data, size);
  return true;
}


void 
VideoPainter::paint() const
{
  if(SDL_mutexP(this->mut) == -1)
    {
      printStatusMessage("Could not lock texture mutex", this->msgpainter);
      return;
    }

  if(this->videotex)
    {
      glBindTexture(GL_TEXTURE_2D, this->videotex);
      // update texture
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                      this->framewidth, this->frameheight, 
                      GL_RGBA, GL_UNSIGNED_BYTE, 
                      this->texdata);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  else
    {
      //fprintf(stderr, "Video texture was not created. Not painting it.\n");
      return;
    }

  if(SDL_mutexV(this->mut) == -1)
    printStatusMessage("Could not unlock texture mutex", this->msgpainter);
}


EdgeVideoPainter::EdgeVideoPainter(TxtAreaPainter *painter,
                           size_t width, 
                           size_t height)
  : VideoPainter(painter, width, height),
    frame_counter(0)
{
}


bool 
EdgeVideoPainter::processImageBuffer(unsigned char *data, size_t size)
{
  ++this->frame_counter;
  //if(this->frame_counter % 5 == 0)
    {
      // Process only every 5-th frame
      Mat edges;
      frameToEdge(data, this->framewidth, this->frameheight, edges);
      memcpy(this->texdata, edges.data, size);
    }
  return false;
}
