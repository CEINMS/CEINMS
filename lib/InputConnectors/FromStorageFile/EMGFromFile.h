#ifndef EMGFromFile_h
#define EMGFromFile_h

#include <string>
#include <vector>
#include <iostream>

#include "EMGFromX.h"

#include "EMGDataFromFile.h"
#include "EMGgeneratorFromXml.h"
#include "InputQueues.h"

#define LOG

class EMGFromFile:public EMGFromX
{
  public:
    template <typename NMSModelT>
    EMGFromFile(const NMSModelT& subject, const std::string& inputDir);
    void operator()();
    virtual ~EMGFromFile();
  private:  
    std::string inputDir_;
    EMGDataFromFile<EMGgeneratorFromXml> emgData_;
};


template <typename NMSModelT>
EMGFromFile::EMGFromFile(const NMSModelT& subject, const std::string& emgFilename)
:EMGFromX(subject), emgData_(emgFilename) 
{ 
   
#ifdef LOG  
  std::cout << "\nEMG: Reading emg file..." << emgFilename << std::endl;
#endif
  
  std::vector<std::string> emgMusclesNames;
  emgData_.getMusclesNames(emgMusclesNames);
  if (!(musclesNames_==emgMusclesNames)) {
    std::cout << "\n\n ERROR: Muscle names from emg file and subject differ!\n";
    exit(EXIT_FAILURE);  
  }
 
}

#endif