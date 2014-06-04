#ifndef ExternalTorqueFromFile_h
#define ExternalTorqueFromFile_h

#include "ExternalTorqueFromX.h"

#include <string>

class ExternalTorqueFromFile:public ExternalTorqueFromX 
{
public:
  template <typename NMSModelT>
  ExternalTorqueFromFile(const NMSModelT& subject, const std::string& dataDirectory);
  void operator()();

private:
  bool externalTorqueFileExist(const std::string& fileName);
  std::string dataDirectory_;
  
};


template <typename NMSModelT>
ExternalTorqueFromFile::ExternalTorqueFromFile(const NMSModelT& subject, const std::string& dataDirectory)
:ExternalTorqueFromX(subject),dataDirectory_(dataDirectory)
{  }

#endif
