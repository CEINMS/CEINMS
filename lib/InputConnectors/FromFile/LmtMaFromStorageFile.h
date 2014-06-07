#ifndef LmtMaFromStorageFile_h
#define LmtMaFromStorageFile_h

#include "LmtMaFromX.h"

#include <string>
#include <vector>

class LmtMaFromStorageFile:public LmtMaFromX {
  public:
    template <typename NMSModelT>
    LmtMaFromStorageFile(const NMSModelT& subject , const std::string& dataDirectory);
    void operator()();
   
  private:    
    std::string dataDirectory_;
    
};

template <typename NMSModelT>
LmtMaFromStorageFile::LmtMaFromStorageFile(const NMSModelT& subject, const std::string& dataDirectory)
:LmtMaFromX(subject),dataDirectory_(dataDirectory)
{ }

#endif
