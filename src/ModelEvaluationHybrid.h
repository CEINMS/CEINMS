//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
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

template <typename NMSmodelT, typename ErrorMinimizerT>
class ModelEvaluationHybrid : public ModelEvaluationBase {

public:
    ModelEvaluationHybrid(NMSmodelT& subject, ErrorMinimizerT& torqueErrorMinimizer, const std::string& outputDir = "./Output");
    void operator()();
    
private:
    NMSmodelT& subject_;
    std::string outputDir_;
    ErrorMinimizerT& torqueErrorMinimizer_;
    std::vector< std::string > dofNames_;
    std::vector< std::string > dofNamesWithExtTorque_;
    unsigned noDof_;
};

#include "ModelEvaluationHybrid.cpp"

#endif
