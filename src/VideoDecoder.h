#ifndef __VIDEODECODER_H
#define __VIDEODECODER_H

#include <gst/gst.h>
#include <string>
#include "VideoPainter.h"

class BufferQueue;


class VideoDecoder
{
 public:
  VideoDecoder();
  ~VideoDecoder();

  int initAndStart(int argc, char **argv, 
		   const char *dec_pipeline,
		   const VideoPainterList &vps, 
		   BufferQueue *buffer_queue);

 public:
  void requestShutdown();

  VideoPainterList video_painters;
  BufferQueue *buffer_queue;
  GstPipeline *pipeline;

 private:
  std::string decoding_pipeline;

  GstElement *appsrc;
};

#endif // __VIDEODECODER_H
