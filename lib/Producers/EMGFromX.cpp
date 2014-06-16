//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

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
#include <iostream>


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

void EMGFromX::setEMGMusclesNames(const vector<string>& emgMusclesNames) {

	SyncTools::Shared::musclesNamesMutex.lock();
	if (SyncTools::Shared::musclesNames.empty())         //musclesNames is a global variable
		SyncTools::Shared::musclesNames = emgMusclesNames; 
	else
		if (emgMusclesNames != SyncTools::Shared::musclesNames) {
		std::cout << "lmt names - emg names\n";
		for(unsigned i = 0; i < emgMusclesNames.size() && i < SyncTools::Shared::musclesNames.size() ; ++i) {
			string emgName(emgMusclesNames.at(i)), lmtName(SyncTools::Shared::musclesNames.at(i)), separator("  -  ");
			if(lmtName != emgName) separator = "  <>  ";
			std::cout << lmtName << separator <<  emgName << std::endl;
		}
		std::cout << "ERROR: muscles names among emg and lmt files are different" << std::endl;
		exit(EXIT_FAILURE);          
    }
	SyncTools::Shared::musclesNamesMutex.unlock();
}

EMGFromX::~EMGFromX() { }



