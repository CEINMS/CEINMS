#include "ExternalTorqueFromX.h"
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

void ExternalTorqueFromX::pushExternalTorqueBack(const vector<double>& newExternalTorqueToPush, unsigned int whichDof)
{ 
  SyncTools::Shared::queueExternalTorqueSemEmpty.wait();  //waits if the buffer is full. if so it waits until the consumer removes at least one item from queueLmt
  SyncTools::Shared::queueExternalTorqueMutex.lock();
  SyncTools::Shared::queueExternalTorque.at(whichDof).push_back(newExternalTorqueToPush);
  SyncTools::Shared::queueExternalTorqueMutex.unlock();
  SyncTools::Shared::queueExternalTorqueSemFull.notify(); //notify that an item has been pushed in the queue
}



void ExternalTorqueFromX::updateExternalTorque(const vector<double>& currentExternalTorqueData, double currentTime, unsigned int whichDof)
{
  vector<double> ExternalTorqueDataToPush = currentExternalTorqueData;
  ExternalTorqueDataToPush.push_back(currentTime); //appends currentTime at the end
  pushExternalTorqueBack(ExternalTorqueDataToPush, whichDof);
 
}



//TODO: i nomi dei dof devono essere un sottoinsieme dei dof dall'xml
void ExternalTorqueFromX::setExternalTorqueDofNames(vector<string> dofNamesWithExtTorqueFromInput)
{
  SyncTools::Shared::dofNamesWithExtTorqueMutex.lock();
  SyncTools::Shared::dofNamesWithExtTorque = dofNamesWithExtTorqueFromInput;
  SyncTools::Shared::dofNamesWithExtTorqueMutex.unlock();

//TODO: forse non è il posto migliore per fare quello che c'è sotto..mmmm
  SyncTools::Shared::queueExternalTorqueMutex.lock();
  SyncTools::Shared::queueExternalTorque.resize(dofNamesWithExtTorqueFromInput.size());  
  SyncTools::Shared::queueExternalTorqueMutex.unlock();
}

void ExternalTorqueFromX::getDofNames(vector<string>& dofNamesFromModel)
{
  SyncTools::Shared::dofNamesSem.wait();
  dofNamesFromModel = SyncTools::Shared::dofNames; //gets dof names from XML model, passed from global variable dofNames  
  SyncTools::Shared::dofNamesSem.notify();
}  

ExternalTorqueFromX::~ExternalTorqueFromX() { }




