/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "ServerThread.h"

static const char* CONFIG_PARAM = "--Ice.Config=cockpit.config";


ServerThread::ServerThread(Ice::Application *a, int ac, char **av)
  : app(a), argc(ac)
{
  size_t len;
  // Args will contain the copy of the command line arguments.  We
  // will use args as an argument for Ice::createProperties() function
  // because this function removes ice-specific arguments which we
  // will need later to call app->main(argc, argv);
  Ice::StringSeq args;

  if(ac > 1)
    {
      this->argv = new char*[this->argc];
      for(int i = 0; i < this->argc; i++)
        {
          len = strlen(av[i]) + 1;
          this->argv[i] = new char[len];
          strncpy(this->argv[i], av[i], len);
          args.push_back(this->argv[i]);
        }
    }
  else
    {
      this->argc = 2;
      this->argv = new char*[this->argc];
      len = strlen(av[0]) + 1;
      this->argv[0] = new char[len];
      this->argv[0] = strncpy(this->argv[0], av[0], len);
      args.push_back(this->argv[0]);
      len = strlen(CONFIG_PARAM) + 1;
      this->argv[1] = new char[len];
      this->argv[1] = strncpy(this->argv[1], 
                              CONFIG_PARAM, 
                              len);
      args.push_back(this->argv[1]);
    }
  
  this->props = Ice::createProperties(args);
}


ServerThread::~ServerThread()
{
  delete[] this->argv;
}


std::string 
ServerThread::getStringProperty(const std::string &prop_name) const
{
  return this->props->getProperty(prop_name);
}


Ice::StringSeq 
ServerThread::getStringListProperty(const std::string &prop_name) const
{
  return this->props->getPropertyAsList(prop_name);
}


void 
ServerThread::run()
{
  app->main(this->argc, this->argv);
}
