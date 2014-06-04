#ifndef LmtMaFromFile_h
#define LmtMaFromFile_h

#include "LmtMaFromX.h"

#include <string>
#include <vector>

class LmtMaFromFile:public LmtMaFromX {
  public:
    template <typename NMSModelT>
    LmtMaFromFile(const NMSModelT& subject , const std::string& dataDirectory);
    void operator()();
   
  private:    
    std::string dataDirectory_;
    
};

template <typename NMSModelT>
LmtMaFromFile::LmtMaFromFile(const NMSModelT& subject, const std::string& dataDirectory)
                   :LmtMaFromX(subject),dataDirectory_(dataDirectory)
{ }

#endif
