#ifndef EMGFromFile_h
#define EMGFromFile_h

#include <string>
#include <vector>

#include "EMGFromX.h"

class EMGFromFile:public EMGFromX
{
  public:
    EMGFromFile(const std::string& dataDirectory);
    void operator()();
    virtual ~EMGFromFile();
  private:  
    void setEMGMusclesNames(const std::vector< std::string >& emgMusclesNames);
    std::string dataDirectory_;
};

#endif
