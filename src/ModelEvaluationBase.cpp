//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "InputConnectors.h"
#include "OutputConnectors.h"

#include <vector>
using std::vector;
#include <string>
using std::string;


namespace ceinms {
    template <typename Logger>
    ModelEvaluationBase<Logger>::ModelEvaluationBase(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, const vector<string>& valuesToLog)
        :inputConnectors_(inputConnectors), outputConnectors_(outputConnectors), logger(outputConnectors, valuesToLog)
    { }


    template <typename Logger>
    void ModelEvaluationBase<Logger>::subscribeToInputConnectors() {

        inputConnectors_.queueLmt.subscribe();
        inputConnectors_.queueEmg.subscribe();
        for (auto& it : inputConnectors_.queueMomentArms)
            (*it).subscribe();
        inputConnectors_.queueExternalTorques.subscribe();

        inputConnectors_.doneWithSubscription.wait();
    }


    template <typename Logger>
    bool ModelEvaluationBase<Logger>::externalTorquesAvailable() const {
        return inputConnectors_.externalTorquesAvailable;
    }

    template <typename Logger>
    float ModelEvaluationBase<Logger>::getGlobalTimeLimit() const {
        return inputConnectors_.globalTimeLimit;
    }

    template <typename Logger>
    void ModelEvaluationBase<Logger>::getEmgFromInputQueue(InputConnectors::FrameType& emgs) {
        emgs = inputConnectors_.queueEmg.pop();
    }


    template <typename Logger>
    void ModelEvaluationBase<Logger>::getLmtFromInputQueue(InputConnectors::FrameType& lmts) {
        lmts = inputConnectors_.queueLmt.pop();
    }


    template <typename Logger>
    void ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(InputConnectors::FrameType& momentArms, unsigned int whichDof) {
        momentArms = (*inputConnectors_.queueMomentArms.at(whichDof)).pop();
    }


    template <typename Logger>
    void ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(InputConnectors::FrameType& externalTorques) {
        externalTorques = inputConnectors_.queueExternalTorques.pop();
    }


    template <typename Logger>
    InputConnectors::FrameType ModelEvaluationBase<Logger>::getEmgFromInputQueue() {
        return inputConnectors_.queueEmg.pop();
    }


    template <typename Logger>
    InputConnectors::FrameType ModelEvaluationBase<Logger>::getLmtFromInputQueue() {
        return inputConnectors_.queueLmt.pop();
    }


    template <typename Logger>
    InputConnectors::FrameType ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(unsigned int whichDof) {
        return (*inputConnectors_.queueMomentArms.at(whichDof)).pop();
    }


    template <typename Logger>
    InputConnectors::FrameType ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(){
        return inputConnectors_.queueExternalTorques.pop();
    }

    template <typename Logger>
    void ModelEvaluationBase<Logger>::doneWithExecution(){
        outputConnectors_.doneWithExecution.wait();
    }



    template <typename Logger>
    ModelEvaluationBase<Logger>::~ModelEvaluationBase() { }


}

