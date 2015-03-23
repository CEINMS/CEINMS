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

#include "InputConnectors.h"
#include "OutputConnectors.h"

template <typename Logger>
class ModelEvaluationBase {
    
public:  
  ModelEvaluationBase() = delete; 
  ModelEvaluationBase(CEINMS::InputConnectors& inputConnectors, CEINMS::OutputConnectors& outputConnectors, const std::vector<std::string>& valuesToLog);
  virtual ~ModelEvaluationBase();
  virtual void operator()() = 0;

protected:
  void getEmgFromInputQueue(CEINMS::InputConnectors::FrameType& emgFromQueue);
  void getLmtFromInputQueue(CEINMS::InputConnectors::FrameType& lmtFromQueue);
  void getMomentArmsFromInputQueue(CEINMS::InputConnectors::FrameType& momentArmsFromQueue, unsigned int whichDof);
  void getExternalTorquesFromInputQueue(CEINMS::InputConnectors::FrameType& externalTorquesFromQueue); 
  void doneWithExecution();

  CEINMS::InputConnectors::FrameType getEmgFromInputQueue();
  CEINMS::InputConnectors::FrameType getLmtFromInputQueue();
  CEINMS::InputConnectors::FrameType getMomentArmsFromInputQueue(unsigned int whichDof);
  CEINMS::InputConnectors::FrameType getExternalTorquesFromInputQueue();

  void subscribeToInputConnectors();
  bool externalTorquesAvailable() const;
  float getGlobalTimeLimit() const;

  Logger logger; 

private:
    CEINMS::InputConnectors& inputConnectors_;
    CEINMS::OutputConnectors& outputConnectors_;
};

#include "ModelEvaluationBase.cpp"
#endif