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
 

#ifdef LOG
  cout << "Starting reading input data about lmg and ma" << endl;
#endif
  
  string pattern{"_MuscleAnalysis_Length.sto"};
  string directory{dataDirectory_ + "/MuscleAnalysis/"};
  string lmtDataFilename = directory + findFile(pattern, directory);
  
#ifdef LOG  
  cout << "\nReading lma data from: " << lmtDataFilename << endl;
#endif
  
  DataFromStorageFile lmtData(lmtDataFilename);
  vector<string> musclesNamesFromStorageFile = lmtData.getColumnNames();
  vector<size_t> musclePositionInStorage =  findMapping(musclesNames_, musclesNamesFromStorageFile); 
  
#ifdef LOG  
  cout << "\nLmt data match model configuration\n";
#endif
  
  vector< DataFromStorageFile* > maDataStorageFiles;
  vector< vector<size_t> > musclePositionsInMomentArmStorage( dofNames_.size() );
  int currentDof = 0; 
  for (auto& it: dofNames_)
  {
    string pattern = "_MomentArm_" + it + ".sto";
    string maDataFileName = directory + findFile(pattern, directory);
#ifdef LOG  
    cout << "\nReading ma data from:" << maDataFileName << endl;
#endif  
    DataFromStorageFile* nextMaDataPtr = new DataFromStorageFile(maDataFileName);
    maDataStorageFiles.push_back(nextMaDataPtr);
    
    vector<string> muscleNamesFromStorageFile = (*maDataStorageFiles.at(currentDof)).getColumnNames();
    musclePositionsInMomentArmStorage.at(currentDof) = findMapping(muscleNamesOnDofs_.at(currentDof), muscleNamesFromStorageFile);
    currentDof++;
  }
  
  for (int i=0; i < dofNames_.size(); ++i) 
    CEINMS::InputConnectors::queueMomentArms.push_back(new CEINMS::Concurrency::Queue< vector<double> >); 
     
  
#ifdef LOG  
  cout << "\nMa data match model configuration\n";
#endif
 
  CEINMS::InputConnectors::inputQueuesAreReady.wait(); 
  
  CEINMS::InputConnectors::doneWithSubscription.wait();
  
  double time;  
  while (lmtData.areStillData())
  {    
    lmtData.readNextData();
    time=lmtData.getCurrentTime(); //lmt and momentArms times are equal, ALWAYS!    
    
    auto newLmtData = lmtData.getCurrentData();
    vector<double> selectedLmtData(musclesNames_.size());
    for (int i=0; i < musclesNames_.size(); ++i)
      selectedLmtData.at(i) = newLmtData.at(musclePositionInStorage.at(i));
    updateLmt(selectedLmtData, time);
    
    for (unsigned int currentDof = 0; currentDof < dofNames_.size(); ++currentDof)
    {
      vector<double> selectedMaData(musclePositionsInMomentArmStorage.at(currentDof).size()); 
      auto newMaData = (*maDataStorageFiles.at(currentDof)).getCurrentData();
      for (int i = 0; i < selectedMaData.size(); ++i)
        selectedMaData.at(i) = newMaData.at(musclePositionsInMomentArmStorage.at(currentDof).at(i));
      updateMomentArms(selectedMaData, time, currentDof);
      }
  }
  
  
 

  vector<double> endOfData;
  updateLmt(endOfData, 0);
  for (unsigned int i = 0; i < dofNames_.size(); ++i)
     updateMomentArms(endOfData, 0, i); 

  //SyncTools::Shared::lmtProducingDone.notify(); //used for validate curve only

#ifdef LOG  
  cout << "\nLmtMa: lmtMa DONE\n";
#endif

}

