/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "GstMainThread.h"
#include "VideoPainter.h"
#include "BufferQueue.h"
#include <SDL.h>
#include <SDL_thread.h>


GST_DEBUG_CATEGORY (cockpit_video_debug);
#define GST_CAT_DEFAULT cockpit_video_debug


int ThreadFunction(void *data)
{
  GstMainThread *mt = (GstMainThread*)data;
  return mt->run();
}


GstMainThread::GstMainThread(int ac, 
                             char **av, 
                             const char *dec_pipeline,
                             VideoPainter *vp,
                             BufferQueue *bq)
  : video_painter(vp), 
    buffer_queue(bq),
    thread(NULL),
    shutdown_requested(false),
    decoding_pipeline(dec_pipeline),
    argc(ac),
    argv(av),
    pipeline(NULL),
    appsrc(NULL),
    loop(NULL)
{
}


GstMainThread* 
GstMainThread::create(int ac, 
                      char **av, 
                      const char *dec_pipeline,
                      VideoPainter *video_painter,
                      BufferQueue *buffer_queue)
{
  const gchar *nano_str;
  guint major, minor, micro, nano;

  gst_init(&ac, &av);

  gst_version(&major, &minor, &micro, &nano);
  if(nano == 1)
    nano_str = "(CVS)";
  else if(nano == 2)
    nano_str = "(Prerelease)";
  else
    nano_str = "";
  printf("This program is linked against GStreamer %d.%d.%d %s\n",
         major, minor, micro, nano_str);

  GST_DEBUG_CATEGORY_INIT (cockpit_video_debug, "cockpit_video", 0,
      "cockpit video system");

  GstMainThread *mt = 
    new GstMainThread(ac, av, dec_pipeline, video_painter, buffer_queue);
  mt->thread = SDL_CreateThread(ThreadFunction, mt);
  return mt;
}


void 
GstMainThread::requestShutdown()
{
  gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_NULL);
  g_main_loop_quit(this->loop);
  gst_object_unref(this->pipeline);
  this->shutdown_requested = true;
}


int 
GstMainThread::join()
{
  int status = 0;
  SDL_WaitThread(this->thread, &status);
  return status;
}


static gboolean
bus_message(GstBus * bus, GstMessage * message, GstMainThread *thread)
{
  GST_DEBUG("got message %s",
            gst_message_type_get_name(GST_MESSAGE_TYPE(message)));

  switch(GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_ERROR:
      {
        GError *error;
        gchar *debug;

        gst_message_parse_error(message,
                                &error,
                                &debug);
        g_print("Error message: %s\n", error->message);
        g_print("Debug message: %s\n", debug);
        g_free(debug);

        g_error("received error");
        thread->requestShutdown();
      }
      break;

    case GST_MESSAGE_EOS:
      thread->requestShutdown();
      break;

    default:
      break;
    }
  return TRUE;
}


/* fakesink handoff callback */
void
on_gst_buffer(GstElement *element,
              GstBuffer *buf,
              GstPad *pad,
              GstMainThread *t)
{
  gst_buffer_ref(buf);
  t->video_painter->receiveBuffer(GST_BUFFER_DATA(buf), GST_BUFFER_SIZE(buf));
  gst_buffer_unref(buf);
}


/* This method is called by the need-data signal callback, we feed
 * data into the appsrc.
 */
static void
feed_data(GstElement *appsrc, guint size, BufferQueue *buffer_queue)
{
  GstBuffer *buffer = buffer_queue->retrieve();
  GstFlowReturn ret;

  if(!buffer)
    {// we are EOS, send end-of-stream
      g_signal_emit_by_name (appsrc, "end-of-stream", &ret);
      return;
    }

  // feed buffer
  g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
  gst_buffer_unref(buffer);

  return;
}


int 
GstMainThread::run()
{
  /* create a mainloop to get messages and to handle the idle handler
   * that will feed data to appsrc. */
  this->loop = g_main_loop_new(NULL, FALSE);

  this->pipeline = 
    GST_PIPELINE(gst_parse_launch(this->decoding_pipeline.c_str(), NULL));

  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));
  // add watch for messages
  gst_bus_add_watch(bus, (GstBusFunc)bus_message, this);

  // set a callback to retrieve the gst gl textures
  GstElement *fakesink = gst_bin_get_by_name(GST_BIN(this->pipeline),
                                             "fakesink0");
  g_object_set(G_OBJECT (fakesink), "signal-handoffs", TRUE, NULL);
  g_signal_connect(fakesink, "handoff", G_CALLBACK(on_gst_buffer), this);
  g_object_unref(fakesink);

  // get a handle to the appsrc
  GstElement *appsrc = gst_bin_get_by_name(GST_BIN(this->pipeline),
                                           "appsrc0");
  g_signal_connect(appsrc, 
                   "need-data", 
                   G_CALLBACK(feed_data), 
                   this->buffer_queue);
  g_object_unref(appsrc);


  GstStateChangeReturn ret =
    gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_PLAYING);
  if(ret == GST_STATE_CHANGE_FAILURE)
    {
      fprintf(stderr, "Failed to start up pipeline!");

      /* check if there is an error message with details on the bus */
      GstMessage* msg = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
      if(msg)
        {
          GError *err = NULL;
          gst_message_parse_error(msg, &err, NULL);
          fprintf(stderr, "ERROR: %s", err->message);
          g_error_free(err);
          gst_message_unref(msg);
        }
      return -1;
    }

  /* this mainloop is stopped when we receive an error or EOS */
  //g_main_loop_run(this->loop);
	while(!this->shutdown_requested)
    SDL_Delay(1000);

  g_main_loop_unref(this->loop);
  gst_object_unref(bus);

  return 0;
}
