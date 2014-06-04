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
  ModelEvaluationBase(); 
  virtual ~ModelEvaluationBase();
  virtual void operator()() = 0;
    
  void getEmgFromInputQueue(std::vector<double>& emgFromQueue);
  void getLmtFromInputQueue(std::vector<double>& lmtFromQueue);
  void getMomentArmsFromInputQueue(std::vector<double>& momentArmsFromQueue, unsigned int whichDof);
  void getExternalTorqueFromInputQueue(std::vector<double>& externalTorqueFromQueue, unsigned int whichDof); 
};

#endif