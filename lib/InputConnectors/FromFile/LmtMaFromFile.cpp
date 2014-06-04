#include "LmtMaFromFile.h"

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

void LmtMaFromFile::operator()()
{
  double myTime;  
  vector<double> endOfData;

#ifdef LOG
  cout << "starting lmtMaProduce, reading from lmt and ma data files" << endl;
#endif

  string lmtDataFilename = dataDirectory_ + "lmt.txt";  
  
#ifdef LOG  
  cout << "\nReading..." << lmtDataFilename << endl;
#endif
  DataFromFile myLmtData(lmtDataFilename);
  vector<string> musclesNamesFromFile;
  musclesNamesFromFile = myLmtData.getMusclesNames();
  
  if (!(musclesNames_ == musclesNamesFromFile)) {
    std::cout << "\n\n ERROR: Muscle names from lmt file and subject differ!\n";
    exit(EXIT_FAILURE);  
  }
  
#ifdef LOG  
  cout << "\nend of lmt INIT\n\n";
#endif

  // START OF MOMENT ARMS INIT
  
  vector<DataFromFile*> myMomentArmsData;
  for (auto& dofIt : dofNames_) 
  {
    dofIt = dataDirectory_ + dofIt + "Ma.txt";
#ifdef LOG  
    cout << "\nReading..." << dofIt << endl;
#endif
    DataFromFile* newDoFDataFromFilePointer = new DataFromFile(dofIt);
    myMomentArmsData.push_back(newDoFDataFromFilePointer);  
  }  
  
  for (unsigned int i = 0; i < dofNames_.size(); ++i)
    if (!(muscleNamesOnDofs_.at(i) == myMomentArmsData.at(i)->getMusclesNames()) ) {
      cout << "\nERROR: muscle names connected to dof: " << dofNames_.at(i) << " differ between subject.xml and input data file\n"; 
      exit(EXIT_FAILURE); 
    }
   
#ifdef LOG  
  cout << "\nend of ma INIT\n";
#endif
  
 
  CEINMS::InputConnectors::readyInputQueues.wait(); 

  while (myLmtData.areStillData())
  {    
    myLmtData.readNextData();
    myTime=myLmtData.getCurrentTime(); //lmt and momentArms times are equal, ALWAYS!    
    updateLmt(myLmtData.getCurrentData(), myTime);
    for (unsigned int i = 0; i < dofNames_.size(); ++i)
    {
      myMomentArmsData.at(i)->readNextData();
      updateMomentArms(myMomentArmsData.at(i)->getCurrentData(), myTime, i);
    }
  }

  updateLmt(endOfData, 0);
  for (unsigned int i = 0; i < dofNames_.size(); ++i)
    updateMomentArms(endOfData, 0, i); 

  //SyncTools::Shared::lmtProducingDone.notify(); //used for validate curve only

#ifdef LOG  
  cout << "\nlmtMa DONE\n\n";
#endif

}

