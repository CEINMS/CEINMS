
#include "EMGFromFile.h"

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
  inputConnectors_.doneWithSubscription.wait();
  
  while (emgData_.areStillData())
  { 
    emgData_.readNextEmgData();
    updateEmg(emgData_.getCurrentData(), emgData_.getCurrentTime());
  } 
  
  vector<double> endOfEmg;
  updateEmg(endOfEmg, std::numeric_limits<double>::infinity());
  
  // SyncTools::Shared::emgProducingDone.notify();
  
#ifdef LOG  
  cout << "\nEMG: EMG DONE\n";
#endif
 
}

EMGFromFile::~EMGFromFile(){}


