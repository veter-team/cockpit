/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "MapDownloadThread.h"
#include "TileManager.h"
#include "TileDownloadCmd.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#else
#include <sys/select.h>
#endif

#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#include <sstream>


static size_t write_data(void *buffer, size_t size, size_t nmemb, void *d)
{
  FILE *f = (FILE*)d;
  fwrite(buffer, size, nmemb, f);
  return size * nmemb;
}


MapDownloadThread::MapDownloadThread()
  : new_handle(false),
    running_handles(0),
    map(NULL)
{
  this->mut = SDL_CreateMutex();
  this->cond = SDL_CreateCond();
  curl_global_init(CURL_GLOBAL_ALL);
  this->multiple_handle = curl_multi_init();
}


MapDownloadThread::~MapDownloadThread()
{
  if(this->mut)
    SDL_DestroyMutex(mut);
  if(this->cond)
    SDL_DestroyCond(cond);
  curl_multi_cleanup(this->multiple_handle);
}


void 
MapDownloadThread::setMapObject(TileManager *m)
{
  this->map = m;
}

int 
MapDownloadThread::requestShutdownAndWait()
{
  {
    SDLMutexGuard l(this->mut);
    this->shutdown_requested = true;
    SDL_CondSignal(this->cond);
  }
  int status;
  SDL_WaitThread(this->thread, &status);
  this->thread = NULL;
  return status;
}


void
MapDownloadThread::signalDownloadComplete(int x, 
                                          int y, 
                                          const std::string &file_name) const
{
  if(this->map)
    {
      TileDownloadCmd *cmd = new TileDownloadCmd(this->map);
      cmd->x = x;
      cmd->y = y;
      cmd->file_name = file_name;
      SDL_Event event;
      event.type = SDL_USEREVENT;
      event.user.code = 1;
      event.user.data1 = cmd;
      event.user.data2 = 0;
      SDL_PushEvent(&event);
    }
}


void 
MapDownloadThread::addDownloadRequest(int zoom, int x, int y)
{
  std::ostringstream file_name;
  file_name << zoom << "-";
  file_name << x << "-";
  file_name << y << ".png";

  struct stat stat_data;
  if(stat(file_name.str().c_str(), &stat_data) == 0)
    {
      if(stat_data.st_size > 0)
        this->signalDownloadComplete(x, y, file_name.str());
      else
        {
          printf("File size of %s is 0. Deleting the file.\n", 
                 file_name.str().c_str());
          unlink(file_name.str().c_str());
          this->addDownloadRequest(zoom, x, y);
        }
      return;
    }

  CURL *handle = curl_easy_init();
  //curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
  //curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
  //curl_easy_setopt(handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
  //curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
  //curl_easy_setopt(handle, CURLOPT_PROXYUSERPWD, "user:password");
  //curl_easy_setopt(handle, CURLOPT_PROXY, "3.249.252.161:88");
  curl_easy_setopt(handle, 
                   CURLOPT_USERAGENT, 
                   "Veter-project driver console");
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
  std::ostringstream os;
  os << "http://tile.openstreetmap.org/";
  os << zoom << "/";
  os << x << "/";
  os << y << ".png";
  curl_easy_setopt(handle, CURLOPT_URL, os.str().c_str());
  //curl_easy_setopt(handle, CURLOPT_WRITEDATA, ff/grab);
  ActiveDownloadInfo adi;
  adi.x = x;
  adi.y = y;
  adi.file_name = file_name.str();
  adi.ostream = fopen(adi.file_name.c_str(), "wb");
  this->active_downloads.insert(std::make_pair(handle, adi));
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, adi.ostream);

  SDLMutexGuard l(this->mut);
  CURLMcode res = curl_multi_add_handle(this->multiple_handle, handle);
  if(res != CURLM_OK)
    fprintf(stderr, "curl_multi_add_handle() returns error\n");
  this->new_handle = true;
  SDL_CondSignal(this->cond);
}


int 
MapDownloadThread::run()
{
  fd_set read_fd_set, write_fd_set, exc_fd_set;
  int max_fd;
  long timeout_msec;
  struct timeval timeout;
  CURLMcode res;
  int msgs_in_queue = 0;
  CURLMsg *msg = NULL;

  while(!this->shutdown_requested)
    {
      {
        SDLMutexGuard l(this->mut);

        while(!this->new_handle && this->running_handles == 0)
          {
            SDL_CondWait(this->cond, this->mut);
            if(this->shutdown_requested)
              return 0;
          }
        this->new_handle = false;

        int curr_running_handles;
        do {
          res = curl_multi_perform(this->multiple_handle, &curr_running_handles);
          //printf("Running handles: %u\n", curr_running_handles);
          if(res != CURLM_OK && res != CURLM_CALL_MULTI_PERFORM)
            {
              fprintf(stderr, 
                      "curl_multi_perform() returns error:\n%s\n", 
                      curl_multi_strerror(res));
              continue;
            }

          if(curr_running_handles < this->running_handles)
            {
              while((msg = curl_multi_info_read(this->multiple_handle, &msgs_in_queue)))
                {
                  if(msg->msg == CURLMSG_DONE)
                    {
                      if(CURLE_OK == msg->data.result)
                        {
                          ActiveDownloadMap::iterator i 
                            = this->active_downloads.find(msg->easy_handle);
                          if(i != this->active_downloads.end())
                            {
                              fclose(i->second.ostream);
                              struct stat stat_data;
                              if(stat(i->second.file_name.c_str(), &stat_data) == 0)
                                {
                                  if(stat_data.st_size > 0)
                                    this->signalDownloadComplete(i->second.x, 
                                                                 i->second.y, 
                                                                 i->second.file_name);
                                  else
                                    {
                                      printf("File size of %s is 0. Deleting the file.\n", 
                                             i->second.file_name.c_str());
                                      unlink(i->second.file_name.c_str());
                                    }
                                }
                              /*
                              printf("Download for %s completed. Active download map size is: %u\n", 
                                     i->second.file_name.c_str(),
                                     this->active_downloads.size() - 1);
                              */
                              this->active_downloads.erase(i);
                            }
                        }
                      else
                        {
                          fprintf(stderr,
                                  "Download failed.\nError %i : %s\n", 
                                  msg->data.result, 
                                  curl_easy_strerror(msg->data.result));
                        }
                      curl_easy_cleanup(msg->easy_handle);
                    }
                }
            }
          this->running_handles = curr_running_handles;
          if(curr_running_handles == 0)
            break;
        } while(res == CURLM_CALL_MULTI_PERFORM);

        if(curr_running_handles == 0)
          continue;

        curl_multi_timeout(this->multiple_handle, &timeout_msec);
        if(timeout_msec == 0)
          continue;
        else if(timeout_msec == -1)
          {
            timeout.tv_sec = 2; // Default timeout 2 seconds
            timeout.tv_usec = 0;
          }
        else
          {
            timeout.tv_sec = timeout_msec / 1000;
            timeout.tv_usec = (timeout_msec % 1000) * 1000;
          }

        FD_ZERO(&read_fd_set);
        FD_ZERO(&write_fd_set);
        FD_ZERO(&exc_fd_set);
        res = curl_multi_fdset(this->multiple_handle, 
                               &read_fd_set, &write_fd_set, &exc_fd_set, 
                               &max_fd); 
        if(res != CURLM_OK && res != CURLM_CALL_MULTI_PERFORM)
          {
            fprintf(stderr, 
                    "curl_multi_fdset() returns error:\n%s\n", 
                    curl_multi_strerror(res));
            continue;
          }

      }

      while(running_handles > 0
            && select(max_fd + 1, 
                      &read_fd_set, &write_fd_set, &exc_fd_set, 
                      &timeout) < 0)
        if(errno != EINTR)
          {
            perror("select() call failed:");
            break;
          }
    }

  return 0;
}
