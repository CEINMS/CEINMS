#include "ExternalTorqueFromFile.h"
#include "DataFromFile.h"
#include "InputQueues.h"


#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>

#define LOG


bool ExternalTorqueFromFile::externalTorqueFileExist(const string& fileName)
{
  std::ifstream dataFile(fileName.c_str());
  bool exist = dataFile.is_open();
  dataFile.close();
  return exist;
}

void ExternalTorqueFromFile::operator()()
{

#ifdef LOG
  cout << "\nstarting externalTorqueProduce, reading from external torque data files" << endl;
#endif
  
  bool externalTorques = true; 
  for (auto& dofIt: dofNames_)
    if (!(externalTorqueFileExist(dataDirectory_ + dofIt + "Torque.txt") ) )
      externalTorques = false; 
  
  vector<DataFromFile*> externalTorqueData;
  if (externalTorques) {  
    for (auto& dofIt:dofNames_) { 
      if(externalTorqueFileExist(dataDirectory_ + dofIt + "Torque.txt")) {
        dofIt = dataDirectory_ + dofIt + "Torque.txt";
#ifdef LOG  
        cout << "\nReading..." << dofIt << endl;
#endif
        DataFromFile* newDoFDataFromFilePointer = new DataFromFile(dofIt);
        externalTorqueData.push_back(newDoFDataFromFilePointer);  
      }  
    }  
  
  }

//END OF EXTERNAL TORQUE INIT
#ifdef LOG  
  if (externalTorques) 
    cout << "\n external Torques available " << endl;
  else
    cout << "\n external Torques not available " << endl; 
  cout << "\nend of external torque INIT";
#endif
  
  // all initialization stuff MUST be placed before this line
  // be sure to call setLmtMusclesNames(...) and setMomentArmsMusclesNames(...) above
    CEINMS::InputConnectors::readyInputQueues.wait(); 
    
  if (externalTorques) {
    
    double myTime;  
    vector<double> endOfData;

    bool stopCondition = false;
    while (!stopCondition)
    {    
      stopCondition = true;
      for (unsigned int i = 0; i < dofNames_.size(); ++i)
      {
//poiché le torque esterne possono essere calcolate con differenti frequenze di campionamento
//e quindi un file può contenere più dati di un'altro, il controllo della 
//presenza di dati viene effettuato su tutti i file
        if(externalTorqueData.at(i)->areStillData())
        {
          externalTorqueData.at(i)->readNextData();
          myTime = externalTorqueData.at(i)->getCurrentTime();
          updateExternalTorque(externalTorqueData.at(i)->getCurrentData(), myTime, i);
        }
      stopCondition = stopCondition && !externalTorqueData.at(i)->areStillData();
      }
    }
    for (unsigned int i = 0; i < dofNames_.size(); ++i)
      updateExternalTorque(endOfData, 0, i); 

  }
  
#ifdef LOG  
    cout << "\nexternalTorque DONE\n";
#endif

}

