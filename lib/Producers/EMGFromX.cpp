#include "EMGFromX.h"
#include "SyncTools.h"

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include <cstdlib>


void EMGFromX::pushEmgBack(const vector<double>& newEmgToPush)
{ 
  SyncTools::Shared::queueEmgSemEmpty.wait();  //waits if the buffer is full. if so it waits until the consumer removes at least one item from queueEmg
  SyncTools::Shared::queueEmgMutex.lock();
  SyncTools::Shared::queueEmg.push_back(newEmgToPush);
  SyncTools::Shared::queueEmgMutex.unlock();
  SyncTools::Shared::queueEmgSemFull.notify(); //notify that an item has been pushed in the queue
}    

void EMGFromX::updateEmg(const vector<double>& currentEmgData, double currentTime)
{
  vector<double> emgDataToPush = currentEmgData;
  emgDataToPush.push_back(currentTime); //appends currentTime at the end
  pushEmgBack(emgDataToPush);
}


EMGFromX::~EMGFromX() { }



