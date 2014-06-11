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

#include "InputQueues.h"

class ModelEvaluationBase {
    
public:  
  ModelEvaluationBase(); 
  virtual ~ModelEvaluationBase();
  virtual void operator()() = 0;
    
  void getEmgFromInputQueue(CEINMS::InputConnectors::FrameType& emgFromQueue);
  void getLmtFromInputQueue(CEINMS::InputConnectors::FrameType& lmtFromQueue);
  void getMomentArmsFromInputQueue(CEINMS::InputConnectors::FrameType& momentArmsFromQueue, unsigned int whichDof);
  void getExternalTorquesFromInputQueue(CEINMS::InputConnectors::FrameType& externalTorquesFromQueue); 
};

#endif