#ifndef __VIDEODECODER_H
#define __VIDEODECODER_H

#include <gst/gst.h>
#include <string>

class VideoPainter;
class BufferQueue;


class VideoDecoder
{
 public:
  VideoDecoder();
  ~VideoDecoder();

  int initAndStart(int argc, char **argv, 
		   const char *dec_pipeline,
		   VideoPainter *video_painter, 
		   BufferQueue *buffer_queue);

 public:
  void requestShutdown();

  VideoPainter *video_painter;
  BufferQueue *buffer_queue;
  GstPipeline *pipeline;

 private:
  std::string decoding_pipeline;

  GstElement *appsrc;
};

#endif // __VIDEODECODER_H
