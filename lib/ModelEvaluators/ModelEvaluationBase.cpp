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

#include <vector>
using std::vector;
#include <string>
using std::string;

namespace ceinms {

    template<typename Logger>
    ModelEvaluationBase<Logger>::ModelEvaluationBase(
        EmgQueue& inputEmgQueue,
        LmtQueue& inputLmtQueue,
        ExternalTorquesQueue& inputExternalTorqueQueue,
        MultipleMomentArmsQueues& inputMomentArmsQueues,
        ScalarLogQueue& outputLogQueue,
        rtb::Concurrency::Latch& doneWithSubscriptions,
        rtb::Concurrency::Latch& doneWithExecution,
        const vector<string>& valuesToLog,
        bool showLog) :

        inputEmgQueue_(inputEmgQueue),
        inputLmtQueue_(inputLmtQueue),
        inputExternalTorqueQueue_(inputExternalTorqueQueue),
        inputMomentArmsQueues_(inputMomentArmsQueues),
        doneWithSubscriptions_(doneWithSubscriptions),
        doneWithExecution_(doneWithExecution),
        logger(outputLogQueue, valuesToLog),
        showLog_(showLog) { }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::subscribeToQueues() {
        inputEmgQueue_.subscribe();
        inputLmtQueue_.subscribe();
        inputExternalTorqueQueue_.subscribe();
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::doneWithSubscriptions() {
        subscribeToQueues();
        doneWithSubscriptions_.wait();
		for (auto& it : inputMomentArmsQueues_)
			(*it).subscribe();
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::getEmgFromInputQueue(std::optional<EmgFrame>& emgs) {
        emgs = inputEmgQueue_.pop();
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::getLmtFromInputQueue(std::optional<LmtFrame>& lmts) {
        lmts = inputLmtQueue_.pop();
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(std::optional<MomentArmsFrame>& momentArms, unsigned int whichDof) {
        momentArms = (*inputMomentArmsQueues_.at(whichDof)).pop();
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(std::optional<ExternalTorquesFrame>& externalTorques) {
        externalTorques = inputExternalTorqueQueue_.pop();
    }

    template<typename Logger>
    std::optional<EmgFrame> ModelEvaluationBase<Logger>::getEmgFromInputQueue() {
        return inputEmgQueue_.pop();
    }

    template<typename Logger>
    std::optional<LmtFrame> ModelEvaluationBase<Logger>::getLmtFromInputQueue() {
        return inputLmtQueue_.pop();
    }

    template<typename Logger>
    std::optional<MomentArmsFrame> ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(unsigned int whichDof) {
        return (*inputMomentArmsQueues_.at(whichDof)).pop();
    }

    template<typename Logger>
    std::optional<ExternalTorquesFrame> ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue() {
        return inputExternalTorqueQueue_.pop();
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::doneWithExecution() {
        doneWithExecution_.wait();
    }

    template<typename Logger>
    ModelEvaluationBase<Logger>::~ModelEvaluationBase() { }
}