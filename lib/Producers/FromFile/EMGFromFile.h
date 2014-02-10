#ifndef EMGFromFile_h
#define EMGFromFile_h

#include <string>
#include <vector>

#include "EMGFromX.h"

class EMGFromFile:public EMGFromX
{
  public:
    EMGFromFile(const std::string& inputDir);
    void operator()();
    virtual ~EMGFromFile();
  private:  
    std::string inputDir_;
};

#endif
