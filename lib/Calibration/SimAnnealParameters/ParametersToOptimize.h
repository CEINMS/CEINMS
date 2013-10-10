//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ParametersToOptimize_h
#define ParametersToOptimize_h
#include <vector>

class ParametersToOptimize {

public:    
    virtual ~ParametersToOptimize() {} 
    virtual int getNoParameters() = 0;
    virtual void getStartingVectorParameters(std::vector<double>& x) = 0;
    virtual void setVectorParameters(const std::vector<double>& x) = 0;
    virtual void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) = 0;
    
};

#endif