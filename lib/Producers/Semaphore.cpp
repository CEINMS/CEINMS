#include "Semaphore.h"
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

Semaphore::Semaphore(unsigned int count)
          : count_(count) { }

void Semaphore::notify()
{
  boost::mutex::scoped_lock lock(mutex_);
  ++count_;
  condition_.notify_one();
}

void Semaphore::wait()
{
  boost::mutex::scoped_lock lock(mutex_);
  while(!count_)
    condition_.wait(lock);
  --count_;
}

Semaphore::~Semaphore() { }
