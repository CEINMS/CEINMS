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
    void ModelEvaluationBase<Logger>::getEmgFromInputQueue(EmgFrame& emgs) {
        // NOTE: I have added this check here for now, because it was hanging in .pop() when end of data was reached
        if (inputEmgQueue_.messagesToRead() >= 1) {
            std::optional<EmgFrame> EmgFrameFromQueue = inputEmgQueue_.pop();
            if (EmgFrameFromQueue) {
                emgs = EmgFrameFromQueue.value();
            }
        }
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::getLmtFromInputQueue(LmtFrame& lmts) {
        std::optional<LmtFrame> lmtsFromQueue = inputLmtQueue_.pop();
        if (lmtsFromQueue) {
            lmts = lmtsFromQueue.value();
        }
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(MomentArmsFrame& momentArms, unsigned int whichDof) {
        std::optional<MomentArmsFrame> momentArmsFromQueue = (*inputMomentArmsQueues_.at(whichDof)).pop();
        if (momentArmsFromQueue) {
            momentArms = momentArmsFromQueue.value();
        }
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(ExternalTorquesFrame& externalTorques) {
        std::optional<ExternalTorquesFrame> externalTorquesFromQueue = inputExternalTorqueQueue_.pop();
        if (externalTorquesFromQueue) {
            externalTorques = externalTorquesFromQueue.value();
        }
    }

    template<typename Logger>
    EmgFrame ModelEvaluationBase<Logger>::getEmgFromInputQueue() {
        std::optional<EmgFrame> EmgFrameFromQueue = inputEmgQueue_.pop();
        EmgFrame emgs;
        if (EmgFrameFromQueue) {
            emgs = EmgFrameFromQueue.value();
        }
        return emgs;
    }

    template<typename Logger>
    LmtFrame ModelEvaluationBase<Logger>::getLmtFromInputQueue() {
        std::optional<LmtFrame> lmtsFromQueue = inputLmtQueue_.pop();
        LmtFrame lmts;
        if (lmtsFromQueue) {
            lmts = lmtsFromQueue.value();
        }
        return lmts;
    }

    template<typename Logger>
    MomentArmsFrame ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(unsigned int whichDof) {
        std::optional<MomentArmsFrame> momentArmsFromQueue = (*inputMomentArmsQueues_.at(whichDof)).pop();
        MomentArmsFrame momentArms;
        if (momentArmsFromQueue) {
            momentArms = momentArmsFromQueue.value();
        }
        return momentArms;
    }

    template<typename Logger>
    ExternalTorquesFrame ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue() {
        std::optional<ExternalTorquesFrame> externalTorquesFromQueue = inputExternalTorqueQueue_.pop();
        ExternalTorquesFrame externalTorques;
        if (externalTorquesFromQueue) {
            externalTorques = externalTorquesFromQueue.value();
        }
        return externalTorques;
    }

    template<typename Logger>
    void ModelEvaluationBase<Logger>::doneWithExecution() {
        doneWithExecution_.wait();
    }

    template<typename Logger>
    ModelEvaluationBase<Logger>::~ModelEvaluationBase() { }
}