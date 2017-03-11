/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): C. Pizzolato, M. Reggiani                                       *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "ceinms/InputConnectors.h"
#include "ceinms/OutputConnectors.h"

#include <vector>
using std::vector;
#include <string>
using std::string;


namespace ceinms {
    template <typename Logger>
    ModelEvaluationBase<Logger>::ModelEvaluationBase(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, const vector<string>& valuesToLog, bool stiffnessEnabled)
        :inputConnectors_(inputConnectors), outputConnectors_(outputConnectors), logger(outputConnectors, valuesToLog), stiffnessEnabled(stiffnessEnabled)
    { }


    template <typename Logger>
    void ModelEvaluationBase<Logger>::subscribeToInputConnectors() {

        inputConnectors_.queueLmt.subscribe();
        inputConnectors_.queueEmg.subscribe();
        for (auto& it : inputConnectors_.queueMomentArms)
            (*it).subscribe();
        for (auto& it : inputConnectors_.queueMomentArmDerivatives)
            (*it).subscribe();
        inputConnectors_.queueExternalTorques.subscribe();

        inputConnectors_.doneWithSubscription.wait();
    }


    template <typename Logger>
    bool ModelEvaluationBase<Logger>::externalTorquesAvailable() const {
        return inputConnectors_.externalTorquesAvailable;
    }

    template <typename Logger>
    bool ModelEvaluationBase<Logger>::momentArmDerivativesAvailable() const {
        return inputConnectors_.momentArmDerivativesAvailable;
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
    void ModelEvaluationBase<Logger>::getMomentArmDerivativesFromInputQueue(InputConnectors::FrameType& momentArmDerivatives, unsigned int whichDof) {
        momentArmDerivatives = (*inputConnectors_.queueMomentArmDerivatives.at(whichDof)).pop();
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
    InputConnectors::FrameType ModelEvaluationBase<Logger>::getMomentArmDerivativesFromInputQueue(unsigned int whichDof) {
        return (*inputConnectors_.queueMomentArmDerivatives.at(whichDof)).pop();
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
