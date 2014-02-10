#ifndef LmtMaFromFile_h
#define LmtMaFromFile_h

#include "LmtMaFromX.h"

#include <string>
#include <vector>


class LmtMaFromFile:public LmtMaFromX {
  public:
    LmtMaFromFile(const std::string& dataDirectory);
    void operator()();
   
  private:
    
    std::string dataDirectory_;
};

#endif
