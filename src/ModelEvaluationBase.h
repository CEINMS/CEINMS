//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
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

#include "InputQueues.h"

template <typename Logger>
class ModelEvaluationBase {
    
public:  
  ModelEvaluationBase() = delete; 
  ModelEvaluationBase(const std::vector<std::string>& valuesToLog); 
  virtual ~ModelEvaluationBase();
  virtual void operator()() = 0;

protected:
  void getEmgFromInputQueue(CEINMS::InputConnectors::FrameType& emgFromQueue);
  void getLmtFromInputQueue(CEINMS::InputConnectors::FrameType& lmtFromQueue);
  void getMomentArmsFromInputQueue(CEINMS::InputConnectors::FrameType& momentArmsFromQueue, unsigned int whichDof);
  void getExternalTorquesFromInputQueue(CEINMS::InputConnectors::FrameType& externalTorquesFromQueue); 
  
  Logger logger; 
};

#include "ModelEvaluationBase.cpp"
#endif