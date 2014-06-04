//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "ModelEvaluationBase.h"
#include "InputQueues.h"

#include <vector>
using std::vector;
#include <string>
using std::string;

ModelEvaluationBase::ModelEvaluationBase() {
  CEINMS::InputConnectors::queueLmt.subscribe();
  CEINMS::InputConnectors::queueEmg.subscribe();
  for (auto& it : CEINMS::InputConnectors::queueMomentArms)
    it.subscribe(); 
  for (auto& it : CEINMS::InputConnectors::queueExternalTorque)
    it.subscribe(); 
}

void ModelEvaluationBase::getEmgFromInputQueue(vector<double>& emgs) {
  emgs = CEINMS::InputConnectors::queueEmg.pop(); 
}

void ModelEvaluationBase::getLmtFromInputQueue(vector<double>& lmts) {
  lmts =  CEINMS::InputConnectors::queueLmt.pop();
}


void ModelEvaluationBase::getMomentArmsFromInputQueue(vector<double>& momentArms, unsigned int whichDof) {
  momentArms =  CEINMS::InputConnectors::queueMomentArms.at(whichDof).pop();
}


void ModelEvaluationBase::getExternalTorqueFromInputQueue(vector<double>& externalTorque, unsigned int whichDof) {
  externalTorque =  CEINMS::InputConnectors::queueExternalTorque.at(whichDof).pop(); 
}

ModelEvaluationBase::~ModelEvaluationBase() { }




