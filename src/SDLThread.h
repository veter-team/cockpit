/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/
#ifndef __SDLTHREAD_H
#define __SDLTHREAD_H

#include <SDL.h>
#include <SDL_thread.h>


class SDLMutexGuard
{
public:
  SDLMutexGuard(SDL_mutex *m) : mutex(m) 
  {  
    if(SDL_mutexP(this->mutex) == -1)
    {
      fprintf(stderr, "Could not SDLMutexGuard mutex");
      return;
    }
  }
  ~SDLMutexGuard()
  {
    if(SDL_mutexV(this->mutex) == -1)
      fprintf(stderr, "Could not unSDLMutexGuard mutex");
  }

private:
  SDL_mutex *mutex;
};


class SDLThread
{
public:
  SDLThread();
  virtual ~SDLThread();

public:
  SDL_Thread* start();
  virtual int run() = 0;
  virtual int requestShutdownAndWait();

protected:
  SDL_Thread *thread;
  bool shutdown_requested;
};


#endif // __SDLTHREAD_H
