#include "ExternalTorquesFromStorageFile.h"
#include "ExternalTorquesFromX.h"
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
#include "InputQueues.h"
#include "Utilities.h"

#define LOG


bool ExternalTorquesFromStorageFile::externalTorquesFileExist(const string& fileName)
{
  std::ifstream dataFile(fileName.c_str());
  bool exist = dataFile.is_open();
  dataFile.close();
  return exist;
}

void ExternalTorquesFromStorageFile::operator()()
{

#ifdef LOG
  cout << "\nExtTorque: starting externalTorqueProduce, reading from external torque data file" << endl;
#endif
  
  CEINMS::InputConnectors::externalTorquesAvailable = externalTorquesFileExist(dataDirectory_ + "inverse_dynamics.sto");
  
  
  if (CEINMS::InputConnectors::externalTorquesAvailable)  {   
#ifdef LOG  
    cout << "\n ExtTorque: external Torques available " << endl;
#endif
    
    DataFromStorageFile externalTorquesData(dataDirectory_ + "inverse_dynamics.sto"); 
    vector<string> columnNamesInStorageFile = externalTorquesData.getColumnNames();
    
    vector<string> torqueNamesToFind; 
    for(auto& it: dofNames_)
      torqueNamesToFind.push_back(it+"_moment");
    
    dofPosInStorage_ = findMapping(torqueNamesToFind, columnNamesInStorageFile); 
    
    CEINMS::InputConnectors::inputQueuesAreReady.wait(); 
    CEINMS::InputConnectors::doneWithSubscription.wait();
    
    while (externalTorquesData.areStillData())
    {
      externalTorquesData.readNextData();
      auto newTorquesData =  externalTorquesData.getCurrentData();
      vector<double> selectedTorquesData(dofNames_.size());
      for (int i = 0; i < dofNames_.size(); ++i) {
        selectedTorquesData.at(i) = newTorquesData.at(dofPosInStorage_.at(i));
      }
      updateExternalTorques(selectedTorquesData, externalTorquesData.getCurrentTime() );
    }
    
    vector<double> endOfTorques;
    updateExternalTorques(endOfTorques, 0); 
    
    } 
    else {
#ifdef LOG  
      cout << "\n ExtTorque: external Torques not available " << endl; 
#endif
      CEINMS::InputConnectors::inputQueuesAreReady.wait(); 
      CEINMS::InputConnectors::doneWithSubscription.wait();
    }

#ifdef LOG  
      cout << "\nexternalTorque DONE\n";
#endif

}

