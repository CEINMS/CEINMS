#include "EMGFromFile.h"
#include "InputQueues.h"
#include "EMGDataFromFile.h"
#include "EMGgeneratorFromXml.h"
#include "FileUtils.h"

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;

#include <cstdlib>

#define LOG

void EMGFromFile::operator()()
{
  string filename(FileUtils::getFile(inputDir_, "emg.txt"));
    
#ifdef LOG  
  cout << "\nEMG: Reading emg file..." << filename << endl;
#endif
  
  EMGDataFromFile<EMGgeneratorFromXml> myEmgData(filename);
  vector<string> emgMusclesNames;
  myEmgData.getMusclesNames(emgMusclesNames);
  if (!(musclesNames_==emgMusclesNames)) {
    std::cout << "\n\n ERROR: Muscle names from emg file and subject differ!\n";
    exit(EXIT_FAILURE);  
  }
  
  CEINMS::InputConnectors::inputQueuesAreReady.wait(); 
  CEINMS::InputConnectors::doneWithSubscription.wait();
  
  while (myEmgData.areStillData())
  { 
    myEmgData.readNextEmgData();
    updateEmg(myEmgData.getCurrentData(), myEmgData.getCurrentTime());
  } 
  vector<double> endOfEmg;
  updateEmg(endOfEmg, 0); 
  
  // SyncTools::Shared::emgProducingDone.notify();
  
#ifdef LOG  
  cout << "\nEMG: EMG DONE\n";
#endif
 
}

EMGFromFile::~EMGFromFile(){}


