#ifndef LmtMaFromOpenSim_h
#define LmtMaFromOpenSim_h

#include "LmtMaFromX.h"
#include "OpenSim/Common/Storage.h"

#include <string>
#include <vector>


class LmtMaFromOpenSim:public LmtMaFromX {
public:
    LmtMaFromOpenSim(const OpenSim::Storage& lmtData, const std::vector<OpenSim::Storage>& maData);
    void operator()();
    void setMusclesOnDofs(const std::vector< std::vector <std::string> >& musclesOnDofs) { musclesOnDofs_ = musclesOnDofs; }
   
private:
    void stripStateData(const std::vector<double>& srcData, std::vector<double>& dstData, unsigned whichDof);
    OpenSim::Storage lmtData_;
    std::vector<OpenSim::Storage> maData_;
    std::vector< std::vector<std::string> > musclesOnDofs_;
    std::vector<std::string> muscleNames_;
};

#endif 
