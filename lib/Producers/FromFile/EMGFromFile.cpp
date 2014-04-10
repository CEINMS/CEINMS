#include "EMGFromFile.h"
#include "SyncTools.h"
#include "EMGDataFromFile.h"
#include "EMGgeneratorFrom16To34.h"
#include "EMGgeneratorFrom6To24.h"
#include "EMGgeneratorFromXml.h"
#include "FileUtils.h"

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;

#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include <cstdlib>

#define LOG

EMGFromFile::EMGFromFile(const string& inputDir):
inputDir_(inputDir)
{ }
               

void EMGFromFile::operator()()
{

	string filename(FileUtils::getFile(inputDir_, "emg.txt"));

#ifdef LOG
  cout << "emg produce" << endl;
#endif
  double myTime;
#ifdef LOG  
  cout << "\nReading emg file..." << filename << endl;
#endif
  EMGDataFromFile<EMGgeneratorFromXml> myEmgData(filename);
  vector<string> emgMusclesNames;
  myEmgData.getMusclesNames(emgMusclesNames);
  setEMGMusclesNames(emgMusclesNames);

  // all initialization stuff MUST be placed before this line
  // be sure to call setEmgMusclesNames(...) above if you want to
  SyncTools::Shared::readyToStart.wait();
  
  while (myEmgData.areStillData())
  { 
    myEmgData.readNextEmgData();
    myTime=myEmgData.getCurrentTime();
    updateEmg(myEmgData.getCurrentData(), myTime);
  } 
  vector<double> endOfEmg;
  updateEmg(endOfEmg, 0); 
  
  SyncTools::Shared::emgProducingDone.notify();
  
#ifdef LOG  
  cout << "\nEMG DONE\n";
#endif
 
}

EMGFromFile::~EMGFromFile(){}


