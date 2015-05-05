//__________________________________________________________________________
// Author(s): Monica Reggiani - June 2014
// email:  monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_Latch_h
#define ceinms_Latch_h

#include <thread>
#include <mutex>
#include <condition_variable>

namespace ceinms{
    namespace Concurrency{
        class Latch {
        public:
            Latch();
            Latch(int count);
            void setCount(int count);
            void wait();
            Latch(const Latch&) = delete;
            Latch& operator=(const Latch&) = delete;
        private:
            int count_;
            std::condition_variable condition_;
            std::mutex mutex_;

        };
    }
}


#endif  