//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "LmtMaFromX.h"
#include "DataFromFile.h"
#include "SyncTools.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <cstdlib>

void LmtMaFromX::pushLmtBack(const vector<double>& newLmtToPush)
{ 
  SyncTools::Shared::queueLmtSemEmpty.wait();  //waits if the buffer is full. if so it waits until the consumer removes at least one item from queueLmt
  SyncTools::Shared::queueLmtMutex.lock();
  SyncTools::Shared::queueLmt.push_back(newLmtToPush);
  SyncTools::Shared::queueLmtMutex.unlock();
  SyncTools::Shared::queueLmtSemFull.notify(); //notify that an item has been pushed in the queue
}

void LmtMaFromX::pushMomentArmsBack(const vector<double>& newMomentArmsToPush, unsigned int whichDof)
{ 
  SyncTools::Shared::queueMomentArmsSemEmpty.wait(); //waits if the buffer is full. if so it waits until the consumer removes at least one item from queueMomentArms
  SyncTools::Shared::queueMomentArmsMutex.lock();
  SyncTools::Shared::queueMomentArms.at(whichDof).push_back(newMomentArmsToPush);
  SyncTools::Shared::queueMomentArmsMutex.unlock();
  SyncTools::Shared::queueMomentArmsSemFull.notify(); //notify that an item has been pushed in the queue
}

void LmtMaFromX::updateLmt(const vector<double>& currentLmtData, double currentTime)
{
  vector<double> lmtDataToPush = currentLmtData;
  lmtDataToPush.push_back(currentTime); //appends currentTime at the end
  pushLmtBack(lmtDataToPush);
}

void LmtMaFromX::updateMomentArms(const vector<double>& currentMomentArmsData, double currentTime, unsigned int whichDof)
{
  vector<double> momentArmsDataToPush = currentMomentArmsData;
  momentArmsDataToPush.push_back(currentTime); //appends currentTime at the end
  pushMomentArmsBack(momentArmsDataToPush, whichDof);
}
 
void LmtMaFromX::getDofNames(vector<string>& dofNamesFromModel)
{
  SyncTools::Shared::dofNamesSem.wait();
  dofNamesFromModel = SyncTools::Shared::dofNames; //gets dof names from XML model, passed from global variable dofNames  
  SyncTools::Shared::dofNamesSem.notify();

//TODO: forse non è il posto migliore per fare quello che c'è sotto..mmmm
  SyncTools::Shared::queueMomentArmsMutex.lock();
  SyncTools::Shared::queueMomentArms.resize(dofNamesFromModel.size());  
  SyncTools::Shared::queueMomentArmsMutex.unlock();
}
  
LmtMaFromX::~LmtMaFromX() { }




