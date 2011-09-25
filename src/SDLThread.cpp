/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/
#include "SDLThread.h"


static int thread_starter(void *param)
{
  SDLThread *t = (SDLThread*)param;
  return t->run();
}


SDLThread::SDLThread()
  : thread(NULL), 
  shutdown_requested(false)
{
}


SDLThread::~SDLThread()
{
  if(this->thread)
    SDL_KillThread(this->thread);
}


int 
SDLThread::requestShutdownAndWait()
{
  this->shutdown_requested = true;
  int status;
  SDL_WaitThread(this->thread, &status);
  this->thread = NULL;
  return status;
}


SDL_Thread* 
SDLThread::start()
{
  this->thread = SDL_CreateThread(thread_starter, this);
  return this->thread;
}
