#ifndef ExternalTorquesFromFile_h
#define ExternalTorquesFromFile_h

#include "ExternalTorquesFromX.h"
#include "InputConnectors.h"
#include "Utilities.h"
#include "DataFromStorageFile.h"

#include <string>

class ExternalTorquesFromStorageFile:public ExternalTorquesFromX 
{
public:
  template <typename NMSModelT>
  ExternalTorquesFromStorageFile(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& dataDirectory);
  void operator()();

private:
  bool externalTorquesFileExist(const std::string& fileName);
  std::string dataDirectory_;
  std::vector<std::size_t> dofPosInStorage_;
  DataFromStorageFile externalTorquesDataFromFile_;
};


template <typename NMSModelT>
ExternalTorquesFromStorageFile::ExternalTorquesFromStorageFile(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& externalTorqueFilename)
:ExternalTorquesFromX(inputConnectors, subject), externalTorquesDataFromFile_(externalTorqueFilename)
{  
  
#ifdef LOG
  std::cout << "\nExtTorque: starting externalTorqueProduce, reading from external torque data file" << std::endl;
#endif
  
  inputConnectors_.externalTorquesAvailable = externalTorquesFileExist(externalTorqueFilename);
 
  if (inputConnectors_.externalTorquesAvailable)  {
#ifdef LOG  
    std::cout << "\n ExtTorque: external Torques available " << std::endl;
#endif
    
    std::vector<std::string> columnNamesInStorageFile = externalTorquesDataFromFile_.getColumnNames();
    std::vector<std::string> torqueNamesToFind; 
    for(auto& it: dofNames_)
      torqueNamesToFind.push_back(it+"_moment");
    
    dofPosInStorage_ = findMapping(torqueNamesToFind, columnNamesInStorageFile); 
  
  }
}
#endif
