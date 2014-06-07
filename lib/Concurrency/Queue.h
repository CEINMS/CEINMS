//__________________________________________________________________________
// Author(s): Monica Reggiani - June 2014
// email:  monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//
#ifndef Queue_h
#define Queue_h


#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace CEINMS{ namespace Concurrency{
template <typename T>
class Queue {
  public:
    
    Queue() = default;
    Queue(const Queue&)= delete;   
    Queue& operator=(const Queue&) = delete; 
    
    void subscribe(); 
    void unsubscribe();
    
    T pop();
    void push(const T& item);
   
    template <typename B>
    friend std::ostream& operator<< (std::ostream& os,  const Queue<B>& queue);
  
  private:
    
    // decided to go with a list so we can trust the iterator. With other containers you can have
    // reallocation that invalidates iterator
    std::list<T> queue_;
    typedef typename std::list<T>::iterator QueueIterator; 
    
    // could be a single map with a structure. But keep in this way cause it helps in function unsubscribe
    std::map< std::thread::id, QueueIterator > subscribersNextRead_; 
    std::map< std::thread::id, int > subscribersMissingRead_;
 
    std::mutex mutex_;
    std::condition_variable cond_;
    
    // utility function used to find the maximum on a map
    static bool pred(const std::pair< std::thread::id, int>& lhs, const std::pair< std::thread::id, int>& rhs);
    bool someoneSlowerThanMe();
};

}; };
#include "Queue.cpp" 


#endif 


  
    