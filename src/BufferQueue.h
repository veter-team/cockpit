/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __BUFFERQUEUE_H
#define __BUFFERQUEUE_H

#include "Queue.h"
#include <gst/gst.h>
#include <comtypes.h>


class BufferQueue
{
 public:
  BufferQueue();

 public:
  void store(const comtypes::ByteSeq &chunk);
  GstBuffer* retrieve();

 private:
  typedef Queue<GstBuffer*> AsyncBufferQueue;
  AsyncBufferQueue received_buffers;
};

#endif // __BUFFERQUEUE_H
