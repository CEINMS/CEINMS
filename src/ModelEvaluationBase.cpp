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

void ModelEvaluationBase::getEmgFromInputQueue(vector<double>& emgs) {
  emgs = CEINMS::InputConnectors::queueEmg.pop(); 
}

void ModelEvaluationBase::getLmtFromInputQueue(vector<double>& lmts) {
  lmts =  CEINMS::InputConnectors::queueLmt.pop();
}

void ModelEvaluationBase::getMomentArmsFromInputQueue(vector<double>& momentArms, unsigned int whichDof) {
  momentArms =  (*CEINMS::InputConnectors::queueMomentArms.at(whichDof)).pop();
}

void ModelEvaluationBase::getExternalTorquesFromInputQueue(vector<double>& externalTorques) {
  externalTorques =  CEINMS::InputConnectors::queueExternalTorques.pop(); 
}

ModelEvaluationBase::~ModelEvaluationBase() { }




