/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __SERVERTHREAD_H
#define __SERVERTHREAD_H

#include <IceUtil/Thread.h>
#include <Ice/Application.h>


class ServerThread : public IceUtil::Thread
{
 public:
  ServerThread(Ice::Application *a, int ac, char **av);
  virtual ~ServerThread();

 public:
  virtual void run();
  std::string getStringProperty(const std::string &prop_name) const;
  Ice::StringSeq getStringListProperty(const std::string &prop_name) const;

 private:
  Ice::Application *app;
  int argc;
  char **argv;
  Ice::PropertiesPtr props;
};

typedef IceUtil::Handle<ServerThread> ServerThreadPtr;

#endif // __SERVERTHREAD_H
