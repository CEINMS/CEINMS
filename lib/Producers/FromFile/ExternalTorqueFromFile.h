#ifndef ExternalTorqueFromFile_h
#define ExternalTorqueFromFile_h

#include "ExternalTorqueFromX.h"

#include <string>

class ExternalTorqueFromFile:public ExternalTorqueFromX {
  public:
    ExternalTorqueFromFile(const std::string& dataDirectory);
    void operator()();
   
  private:
    bool externalTorqueFileExist(const std::string& fileName);
    std::string dataDirectory_;
};

#endif
