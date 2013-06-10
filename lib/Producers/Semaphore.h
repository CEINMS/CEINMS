#ifndef Semaphore_h
#define Semaphore_h

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

//since boost libraries don't provide for semahpores, a simple class, based on mutexes and condition variables, has been made to let the code be clearer
  
class Semaphore
{
public:
    Semaphore(unsigned int count);
    ~Semaphore();    
    void notify();
    void wait();

private:
    boost::mutex mutex_;
    boost::condition_variable condition_;
    unsigned int count_;
};
#endif

