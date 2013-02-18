/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __BUFFERQUEUE_H
#define __BUFFERQUEUE_H

#include "Queue.h"
#include <gst/gst.h>
#include <Ice/BuiltinSequences.h>


class BufferQueue
{
 public:
  BufferQueue();
  ~BufferQueue();

 public:
  void store(const Ice::ByteSeq &chunk);
  GstBuffer* retrieve();

 private:
  typedef Queue<GstBuffer*> AsyncBufferQueue;
  AsyncBufferQueue received_buffers;
  bool destroying;
};

#endif // __BUFFERQUEUE_H
