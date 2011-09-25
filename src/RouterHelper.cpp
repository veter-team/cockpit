/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "RouterHelper.h"


RouterHelper::RouterHelper(const Ice::CommunicatorPtr& communicator)
{
  Ice::RouterPrx r = communicator->getDefaultRouter();
  if(r)
    {
      this->router = Glacier2::RouterPrx::checkedCast(r);
      if(!this->router)
        {
          throw "Wrong interface for router";
        }
      //std::string name;
      //std::string password;
      this->router->createSessionFromSecureConnection();
      this->category = this->router->getCategoryForClient();
    }
}


RouterHelper::~RouterHelper()
{
  try
    {
      if(this->router)
        {
          this->router->destroySession();
        }
    }
  catch(...)
    { // This is intentionaly empty.
    } // We expecting exception here and should not react on it.
}


Ice::Identity 
RouterHelper::makeId(const std::string& name)
{
  Ice::Identity id;
  id.name = name;
  id.category = this->category;
  return id;
}
