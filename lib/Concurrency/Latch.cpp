//__________________________________________________________________________
// Author(s): Monica Reggiani - June 2014
// email:  monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include <iostream>
#include <algorithm>

#include "Latch.h"


  
namespace CEINMS{ namespace Concurrency{
Latch::Latch(int count)
:count_(count) 
{ }

void Latch::wait() {
    std::unique_lock<std::mutex> mlock(mutex_);
    if (count_ == 0) {
      throw std::logic_error("internal count == 0");
    }
    if (--count_ == 0) 
      condition_.notify_all();
    else {
      while(count_ > 0) 
        condition_.wait(mlock);
    }
    mlock.unlock();
}

}; };