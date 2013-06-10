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
    void setLmtMusclesNames(const std::vector< std::string >& lmtMusclesNames);
    void setMomentArmsMusclesNames(const std::vector< std::vector < std::string > >& musclesNamesFromMomentArmsFiles);
    std::string dataDirectory_;
};

#endif
