//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ModelEvaluationOnline_h
#define ModelEvaluationOnline_h

#include <vector>
#include <string>
#include "ModelEvaluationBase.h"


template <typename NMSmodelT>
class ModelEvaluationOnline : public ModelEvaluationBase {

public:
//    ModelEvaluationOnline();
    ModelEvaluationOnline(NMSmodelT& subject);
//    void setSubject(NMSmodelT& subject);
    void operator()();
    
private:
    NMSmodelT& subject_;
 //   std::vector< std::string > musclesNames_;
 //   std::vector< std::vector < std::string > > musclesNamesOnDof_;
    std::vector< std::string > dofNames_;
    std::vector< std::string > dofNamesWithExtTorque_;
    unsigned noDof_;
};

#include "ModelEvaluationOnline.cpp"

#endif