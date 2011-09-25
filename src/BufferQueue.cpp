/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "BufferQueue.h"


BufferQueue::BufferQueue() : received_buffers(5)
{
}


void 
BufferQueue::store(const comtypes::ByteSeq &chunk)
{
  GstBuffer *buffer = gst_buffer_new();

  GST_BUFFER_SIZE(buffer) = chunk.size();
  GST_BUFFER_MALLOCDATA(buffer) = (guint8*)g_malloc(GST_BUFFER_SIZE(buffer));
  GST_BUFFER_DATA(buffer) = GST_BUFFER_MALLOCDATA(buffer);
  memcpy(GST_BUFFER_MALLOCDATA(buffer),
	 &(chunk[0]), 
	 GST_BUFFER_SIZE(buffer));

  this->received_buffers.put(buffer);
}


GstBuffer* 
BufferQueue::retrieve()
{
  GstBuffer *b = NULL;
  
  try
  {
    b = this->received_buffers.get();
  }
  catch(const std::runtime_error &e)
  {
    // we did not receive new frames since some time
    // probably connection to the video server is lost
    fprintf(stderr, "%s\n", e.what());
    return NULL;
  }
  
  return b;
}
