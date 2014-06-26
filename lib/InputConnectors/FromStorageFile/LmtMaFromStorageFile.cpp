#include "LmtMaFromStorageFile.h"

#include "DataFromStorageFile.h"
#include "InputQueues.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>

#include "Utilities.h"

#define LOG


void LmtMaFromStorageFile::operator()()
{
  // 1. wait - if required - for the required subscriptions to happen
  CEINMS::InputConnectors::doneWithSubscription.wait();  
  
  double time;  
  while (lmtData_.areStillData())
  {    
    lmtData_.readNextData();
    auto newLmtData = lmtData_.getCurrentData();
    time=lmtData_.getCurrentTime(); //lmt and momentArms times are equal, ALWAYS!    
    
    vector<double> selectedLmtData(musclesNames_.size());
    for (int i=0; i < musclesNames_.size(); ++i)
      selectedLmtData.at(i) = newLmtData.at(musclePositionInLmtStorage_.at(i));

    updateLmt(selectedLmtData, time);
    
    for (unsigned int currentDof = 0; currentDof < dofNames_.size(); ++currentDof)
    {
      
      (*maDataStorageFiles_.at(currentDof)).readNextData();
      auto newMaData = (*maDataStorageFiles_.at(currentDof)).getCurrentData();
      
      vector<double> selectedMaData(musclePositionsInMaStorages_.at(currentDof).size()); 
      for (int i = 0; i < selectedMaData.size(); ++i)
        selectedMaData.at(i) = newMaData.at(musclePositionsInMaStorages_.at(currentDof).at(i)); 
      
      updateMomentArms(selectedMaData, time, currentDof);
    }
  }
    
  vector<double> endOfData;
  updateLmt(endOfData, std::numeric_limits<double>::infinity());
  for (unsigned int i = 0; i < dofNames_.size(); ++i)
      updateMomentArms(endOfData, std::numeric_limits<double>::infinity(), i);

  //SyncTools::Shared::lmtProducingDone.notify(); //used for validate curve only
 
    
#ifdef LOG  
  cout << "\nLmtMa: lmtMa DONE\n";
#endif
  
}
  
  
 

