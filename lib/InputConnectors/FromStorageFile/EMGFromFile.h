#ifndef EMGFromFile_h
#define EMGFromFile_h

#include <string>
#include <vector>
#include <iostream>

#include "EMGFromX.h"

#include "EMGDataFromFile.h"
#include "EMGgeneratorFromXml.h"
#include "InputConnectors.h"

#define LOG

class EMGFromFile:public EMGFromX
{
  public:
    template <typename NMSModelT>
    EMGFromFile(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& emgFilename, const std::string& emgGeneratorFilename);
    void operator()();
    virtual ~EMGFromFile();
  private:  
    std::string inputDir_;
    EMGDataFromFile<EMGgeneratorFromXml> emgData_;
};


template <typename NMSModelT>
EMGFromFile::EMGFromFile(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& emgFilename, const std::string& emgGeneratorFilename)
:EMGFromX(inputConnectors, subject), emgData_(emgFilename, emgGeneratorFilename)
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
