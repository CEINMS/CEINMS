//__________________________________________________________________________
// Author(s): Monica Reggiani - June 2014
// email:  monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef Latch_h
#define Latch_h

#include <thread>
#include <mutex>
#include <condition_variable>

namespace CEINMS{ namespace Concurrency{
class Latch {
public:
  Latch(int count);

  void wait();

  Latch(const Latch&) = delete;
  Latch& operator=(const Latch&) = delete; 
private:

  int count_;

  std::condition_variable condition_;
  std::mutex mutex_;

};


}; };


#endif  