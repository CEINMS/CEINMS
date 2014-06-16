//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ModelEvaluationBase_h
#define ModelEvaluationBase_h

#include <vector>
#include <string>

class ModelEvaluationBase {
    
public:  
    virtual ~ModelEvaluationBase();
    virtual void operator()() = 0;
    
    void getEmgFromShared(std::vector<double>& emgFromQueue);
    void getLmtFromShared(std::vector<double>& lmtFromQueue);
    void getMomentArmsFromShared(std::vector<double>& momentArmsFromQueue, unsigned int whichDof);
    void getExternalTorqueFromShared(std::vector<double>& externalTorqueFromQueue, unsigned int whichDof);    
    void getMusclesNamesFromShared(std::vector<std::string>& muscleNames);
    void getMusclesNamesOnDofsFromShared(std::vector< std::vector<std::string> >& muscleNamesOnDofs);
    void setDofNamesToShared(const std::vector<std::string>& dofNames);
    void getDofNamesAssociatedToExternalTorque(std::vector<std::string>& dofNames);
};

#endif