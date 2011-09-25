/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __GSTMAINTHREAD_H
#define __GSTMAINTHREAD_H

#include <gst/gst.h>
#include <string>

struct SDL_Thread;
class VideoPainter;
class BufferQueue;


class GstMainThread
{
 public:
  GstMainThread(int ac, char **av, 
                const char *dec_pipeline,
                VideoPainter *vp,
                BufferQueue *bq);

  static GstMainThread* create(int ac, char **av, 
                               const char *dec_pipeline,
                               VideoPainter *video_painter, 
                               BufferQueue *buffer_queue);

 public:
  int run();
  int join();
  void requestShutdown();

  VideoPainter *video_painter;

 private:
  BufferQueue *buffer_queue;
  SDL_Thread *thread;
  bool shutdown_requested;
  std::string decoding_pipeline;
  int argc;
  char **argv;

  GstPipeline* pipeline;
  GstElement *appsrc;
  GMainLoop *loop;
};

#endif // __GSTMAINTHREAD_H
