//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - August 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ModelEvaluationHybrid_h
#define ModelEvaluationHybrid_h

#include <vector>
#include <string>
#include "ModelEvaluationBase.h"

template <typename NMSmodelT, typename ErrorMinimizerT, typename Logger>
class ModelEvaluationHybrid : public ModelEvaluationBase<Logger> {

public:
    ModelEvaluationHybrid(CEINMS::InputConnectors& inputConnectors, NMSmodelT& subject, ErrorMinimizerT& torqueErrorMinimizer, const vector<string>& valuesToLog);
    void setOutputDirectory(const std::string& outputDir);
    void operator()();
    
private:
    NMSmodelT& subject_;
    std::string outputDir_;
    ErrorMinimizerT& torqueErrorMinimizer_;
    std::vector< std::string > dofNames_;
    std::vector< std::string > dofNamesWithExtTorque_;
    unsigned noDof_;
    double globalEmDelay_;

};

#include "ModelEvaluationHybrid.cpp"

#endif
