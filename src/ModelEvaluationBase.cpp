//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "InputQueues.h"

#include <vector>
using std::vector;
#include <string>
using std::string;


template <typename Logger>
ModelEvaluationBase<Logger>::ModelEvaluationBase(const vector<string>& valuesToLog) 
:logger(valuesToLog)
{ }


template <typename Logger>
void ModelEvaluationBase<Logger>::getEmgFromInputQueue(CEINMS::InputConnectors::FrameType& emgs) {
    emgs = CEINMS::InputConnectors::queueEmg.pop(); 
}


template <typename Logger>
void ModelEvaluationBase<Logger>::getLmtFromInputQueue(CEINMS::InputConnectors::FrameType& lmts) {
    lmts =  CEINMS::InputConnectors::queueLmt.pop();
}


template <typename Logger>
void ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(CEINMS::InputConnectors::FrameType& momentArms, unsigned int whichDof) {
    momentArms =  (*CEINMS::InputConnectors::queueMomentArms.at(whichDof)).pop();
}


template <typename Logger>
void ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(CEINMS::InputConnectors::FrameType& externalTorques) {
    externalTorques =  CEINMS::InputConnectors::queueExternalTorques.pop(); 
}


template <typename Logger>
CEINMS::InputConnectors::FrameType ModelEvaluationBase<Logger>::getEmgFromInputQueue() {
    return CEINMS::InputConnectors::queueEmg.pop();
}


template <typename Logger>
CEINMS::InputConnectors::FrameType ModelEvaluationBase<Logger>::getLmtFromInputQueue() {
    return CEINMS::InputConnectors::queueLmt.pop();
}


template <typename Logger>
CEINMS::InputConnectors::FrameType ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(unsigned int whichDof) {
    return (*CEINMS::InputConnectors::queueMomentArms.at(whichDof)).pop();
}


template <typename Logger>
CEINMS::InputConnectors::FrameType ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(){
    return CEINMS::InputConnectors::queueExternalTorques.pop();
}


template <typename Logger>
ModelEvaluationBase<Logger>::~ModelEvaluationBase() { }




