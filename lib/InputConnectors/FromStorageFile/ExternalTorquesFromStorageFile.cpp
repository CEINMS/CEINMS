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

  CEINMS::InputConnectors::doneWithSubscription.wait();
    
  if (CEINMS::InputConnectors::externalTorquesAvailable) 
  {
    while (externalTorquesDataFromFile_.areStillData())
    {
      externalTorquesDataFromFile_.readNextData();
      auto newTorquesData =  externalTorquesDataFromFile_.getCurrentData();
      vector<double> selectedTorquesData(dofNames_.size());
      for (int i = 0; i < dofNames_.size(); ++i) {
        selectedTorquesData.at(i) = newTorquesData.at(dofPosInStorage_.at(i));
      }
      updateExternalTorques(selectedTorquesData, externalTorquesDataFromFile_.getCurrentTime() );
    }
    
    vector<double> endOfTorques;
    updateExternalTorques(endOfTorques, std::numeric_limits<double>::infinity()); 
    
    } 
    else {
#ifdef LOG  
      cout << "\n ExtTorque: external Torques not available " << endl; 
#endif
    }

#ifdef LOG  
      cout << "\nexternalTorque DONE\n";
#endif

}

