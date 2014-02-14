#ifndef ExternalTorqueFromStorage_h
#define ExternalTorqueFromStorage_h

#include "ExternalTorqueFromX.h"

#include <string>
#include <vector>

class ExternalTorqueFromStorage:public ExternalTorqueFromX {
  public:
    ExternalTorqueFromStorage(const std::string& filename);
    void operator()();
   
  private:
    bool externalTorqueFileExist(const std::string& fileName);
    std::vector<int> torqueInStorageIdx_;
    std::string filename_;
};

#endif
