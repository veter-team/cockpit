/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __CONNECTIONTHREAD_H
#define __CONNECTIONTHREAD_H


#include <IceUtil/Thread.h>
#include <vehicle.h>

class IceApp;

class ConnectionThread : public IceUtil::Thread
{
 public:
  ConnectionThread(IceApp *a, 
                   bool *shutdown_flag, 
                   const std::string &unitproxy,
                   vehicle::SensorFrameReceiverPrx &vc);
  virtual ~ConnectionThread();

 public:
  virtual void run();

 private:
  IceApp *app;
  bool *shutdown_requested;
  const std::string unit_str_proxy;
  vehicle::SensorFrameReceiverPrx sensor_callback;
};

typedef IceUtil::Handle<ConnectionThread> ConnectionThreadPtr;


#endif // __CONNECTIONTHREAD_H
