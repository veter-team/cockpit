#ifndef __QUEUE_H
#define __QUEUE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <list>
#include <stdexcept>
#include <stdint.h>


template<class T> 
class Queue : public IceUtil::Monitor<IceUtil::Mutex>
{
 public:
 Queue(IceUtil::Int64 wait_timeout_seconds = 5) 
   : timeout(IceUtil::Time::seconds(wait_timeout_seconds)),
    destroying(false), waiting_threads(0) { }

  virtual ~Queue()
  {
    this->destroying = true;
    while(waiting_threads)
      {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	this->notify();
      }
  }


 public:
  void put(const T& item)
  {
    if(this->destroying)
      return;

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    this->q.push_back(item);
    if(this->q.size() == 1)
      this->notify();
  }

  T get()
  {
    ++waiting_threads;
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    while(this->q.size() == 0 && !destroying)
      {
	//this->wait();
	if(false == this->timedWait(this->timeout))
	  {
	    --waiting_threads;
	    throw std::runtime_error("timeout in Queue::get()");
	  }
      }
    if(this->destroying)
      {
	--waiting_threads;
	return T();
      }

    T item = this->q.front();
    this->q.pop_front();
    --waiting_threads;
    return item;
  }

  typename std::list<T>::size_type size() const 
    {
      IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
      return this->q.size();
    }

 private:
  const IceUtil::Time timeout;
  bool destroying;
  uint8_t waiting_threads;
  std::list<T> q;
};

#endif // __QUEUE_H 
