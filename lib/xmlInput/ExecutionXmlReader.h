//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ExecutionXmlReader_h
#define ExecutionXmlReader_h

#include <string>
#include <vector>
#include "NMSmodelConfig.h"
#include "execution.hxx"


class ExecutionXmlReader {
    
public:
    ExecutionXmlReader(const std::string& filename);
    NMSModelCfg::RunMode getRunMode() const;
    double getTolerance() const { return tolerance_; };
    void getMusclesToPredict(std::vector<std::string>& musclesToPredict);
    void getMusclesToTrack(std::vector<std::string>& musclesToTrack);
    void getHybridWeightings(double& alpha, double& beta, double& gamma);
	void getAnnealingParameters(unsigned& nt, unsigned& ns, double& rt, double& t, unsigned& maxNoEval, double& epsilon, unsigned& noEpsilon) const;
    
private:
    void readXml();
    double tolerance_;
    unsigned runMode_;
    std::auto_ptr<ExecutionType> executionPointer_;
};

#endif