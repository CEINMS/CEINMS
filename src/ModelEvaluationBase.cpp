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
}

void ModelEvaluationBase::getEmgFromInputQueue(CEINMS::InputConnectors::FrameType& emgs) {
  emgs = CEINMS::InputConnectors::queueEmg.pop(); 
}

void ModelEvaluationBase::getLmtFromInputQueue(CEINMS::InputConnectors::FrameType& lmts) {
  lmts =  CEINMS::InputConnectors::queueLmt.pop();
}

void ModelEvaluationBase::getMomentArmsFromInputQueue(CEINMS::InputConnectors::FrameType& momentArms, unsigned int whichDof) {
  momentArms =  (*CEINMS::InputConnectors::queueMomentArms.at(whichDof)).pop();
}

void ModelEvaluationBase::getExternalTorquesFromInputQueue(CEINMS::InputConnectors::FrameType& externalTorques) {
  externalTorques =  CEINMS::InputConnectors::queueExternalTorques.pop(); 
}

ModelEvaluationBase::~ModelEvaluationBase() { }




