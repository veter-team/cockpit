/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __ROUTERHELPER_H
#define __ROUTERHELPER_H

#include <IceUtil/Shared.h>
#include <Glacier2/Router.h>
#include <Ice/Communicator.h>
#include <string>


class RouterHelper : public IceUtil::Shared
{
 public:
  RouterHelper(const Ice::CommunicatorPtr& communicator);
  ~RouterHelper();

 public:
  Ice::Identity makeId(const std::string& name);
  Glacier2::RouterPrx getRouter() { return this->router; }

 private:
  Glacier2::RouterPrx router;
  std::string category;
};

typedef IceUtil::Handle<RouterHelper> RouterHelperPtr;


#endif // __ROUTERHELPER_H
