/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "VideoDecoder.h"
#include "BufferQueue.h"
#include <SDL.h>


GST_DEBUG_CATEGORY (cockpit_video_debug);
#define GST_CAT_DEFAULT cockpit_video_debug


VideoDecoder::VideoDecoder()
  : buffer_queue(NULL),
    pipeline(NULL),
    decoding_pipeline(""),
    appsrc(NULL)
{
}


VideoDecoder::~VideoDecoder()
{
  this->requestShutdown();
}


/* fakesink handoff callback */
void
on_gst_buffer(GstElement *element,
              GstBuffer *buf,
              GstPad *pad,
              VideoDecoder *t)
{
  gst_buffer_ref(buf);
  for(VideoPainterList::iterator vp = t->video_painters.begin();
      vp != t->video_painters.end(); ++vp)
    {
      (*vp)->receiveBuffer(GST_BUFFER_DATA(buf), GST_BUFFER_SIZE(buf));
    }
  gst_buffer_unref(buf);
}


/* This method is called by the need-data signal callback, we feed
 * data into the appsrc.
 */
static void
feed_data(GstElement *appsrc, guint size, VideoDecoder *decoder)
{
  GstBuffer *buffer = decoder->buffer_queue->retrieve();
  GstFlowReturn ret;

  if(!buffer)
    {// we are EOS, send end-of-stream
      g_signal_emit_by_name(appsrc, "end-of-stream", &ret);
      return;
    }

  // feed buffer
  g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
  gst_buffer_unref(buffer);

  return;
}


static gboolean
bus_message(GstBus * bus, GstMessage * message, VideoDecoder *thread)
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


int 
VideoDecoder::initAndStart(int argc, 
			   char **argv, 
			   const char *dec_pipeline,
			   const VideoPainterList &vps,
			   BufferQueue *buffer_queue)
{
  const gchar *nano_str;
  guint major, minor, micro, nano;

  gst_init(&argc, &argv);

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

  this->decoding_pipeline = dec_pipeline;
  this->video_painters = vps;
  this->buffer_queue = buffer_queue;

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
                   this);
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

  gst_object_unref(bus);

  return 0;
}


void 
VideoDecoder::requestShutdown()
{
  if(this->pipeline == NULL || this->appsrc == NULL)
    return;

  GstFlowReturn ret;
  g_signal_emit_by_name(this->appsrc, "end-of-stream", &ret);
  gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_NULL);
  gst_object_unref(this->pipeline);
  this->pipeline = NULL;
  this->appsrc = NULL;
}
