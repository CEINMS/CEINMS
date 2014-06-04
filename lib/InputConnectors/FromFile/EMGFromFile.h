#ifndef EMGFromFile_h
#define EMGFromFile_h

#include <string>
#include <vector>

#include "EMGFromX.h"

class EMGFromFile:public EMGFromX
{
  public:
    template <typename NMSModelT>
    EMGFromFile(const NMSModelT& subject, const std::string& inputDir);
    void operator()();
    virtual ~EMGFromFile();
  private:  
    std::string inputDir_;
};


template <typename NMSModelT>
EMGFromFile::EMGFromFile(const NMSModelT& subject, const std::string& inputDir)
:EMGFromX(subject), inputDir_(inputDir)
{ }

#endif
