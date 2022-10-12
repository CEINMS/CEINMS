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

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include "ceinms/TimeCompare.h"
//#include "SimpleFileLogger.h"
//#include "StorageLogger.h"

#define LOG_FILES
#define LOG

namespace ceinms {

    template<typename NMSmodelT, typename ErrorMinimizerT, typename Logger>
    ModelEvaluationHybrid<NMSmodelT, ErrorMinimizerT, Logger>::ModelEvaluationHybrid(
        EmgQueue& inputEmgQueue,
        LmtQueue& inputLmtQueue,
        ExternalTorquesQueue& inputExternalTorqueQueue,
        MultipleMomentArmsQueues& inputMomentArmsQueues,
        ScalarLogQueue& outputLogQueue,
        rtb::Concurrency::Latch& doneWithSubscriptions,
        rtb::Concurrency::Latch& doneWithExecution,
        NMSmodelT& subject,
        ErrorMinimizerT& torqueErrorMinimizer,
        const vector<string>& valuesToLog
        ) :
        ModelEvaluationBase<Logger>(
        inputEmgQueue,
        inputLmtQueue,
        inputExternalTorqueQueue,
        inputMomentArmsQueues,
        outputLogQueue,
        doneWithSubscriptions,
        doneWithExecution,
        valuesToLog),
        subject_(subject),
        torqueErrorMinimizer_(torqueErrorMinimizer) {

        subject_.getDoFNames(dofNames_);
        noDof_ = dofNames_.size();
        globalEmDelay_ = subject.getGlobalEmDelay();
    }

    template<typename NMSmodelT, typename ErrorMinimizerT, typename Logger>
    void ModelEvaluationHybrid<NMSmodelT, ErrorMinimizerT, Logger>::operator()() {
        ModelEvaluationBase<Logger>::doneWithSubscriptions();

#ifdef LOG
        cout << "starting consume" << endl;
#endif

        double externalTorqueTime = std::numeric_limits<double>::lowest();
        double emgTime = std::numeric_limits<double>::lowest();
        bool runCondition = true;
        bool firstLmtArrived(false);

        std::vector<double> lmtData;
        std::vector<std::vector<double>> momentArmsData;
        std::vector<double> externalTorqueData;
        std::vector<double> emgData;

        do {  // while(runCondition)
            // 1. read lmt Data
            std::optional<LmtFrame> lmtFrameFromQueue;
            ModelEvaluationBase<Logger>::getLmtFromInputQueue(lmtFrameFromQueue);
            double lmtMaTime;
            if (lmtFrameFromQueue) {
                lmtMaTime = lmtFrameFromQueue.value().time;
                lmtData = lmtFrameFromQueue.value().data;
            }
            else {
                lmtMaTime = std::numeric_limits<double>::lowest();
                runCondition = false;
            }

            // 2. read moment arms data
            vector<std::optional<MomentArmsFrame>> momentArmsFrameFromQueue;
            momentArmsFrameFromQueue.resize(noDof_);
            momentArmsData.resize(noDof_);
            for (unsigned int i = 0; i < noDof_; ++i) {
                ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue((momentArmsFrameFromQueue.at(i)), i);
                if (momentArmsFrameFromQueue.at(i)) {
                    momentArmsData.at(i) = momentArmsFrameFromQueue.at(i).value().data;
                }
                else {
                    runCondition = false;
                }
            }

            // 3. read external Torque
            std::optional<ExternalTorquesFrame> externalTorquesFrameFromQueue;
            while (TimeCompare::less(externalTorqueTime, lmtMaTime) && runCondition) {
                ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(externalTorquesFrameFromQueue);
                if (externalTorquesFrameFromQueue) {
                    externalTorqueTime = externalTorquesFrameFromQueue.value().time;
                    externalTorqueData = externalTorquesFrameFromQueue.value().data;
                }
                else {
                    runCondition = false;
                }
            }

            // 4. read emgs
            std::optional<EmgFrame> emgFrameFromQueue;
            while (TimeCompare::less(emgTime, lmtMaTime) && runCondition) {
                ModelEvaluationBase<Logger>::getEmgFromInputQueue(emgFrameFromQueue);
                if (emgFrameFromQueue) {
                    emgTime = emgFrameFromQueue.value().time + globalEmDelay_;
                    emgData = emgFrameFromQueue.value().data;
                }
                else {
                    runCondition = false;
                }
                if (!TimeCompare::less(emgTime, lmtMaTime)) firstLmtArrived = true;
                if (!firstLmtArrived && runCondition) {
                    subject_.setTime(emgTime);
                    subject_.setEmgs(emgFrameFromQueue.value().data);
                }
            }

            //5. lmt, ma, emg, extTorques have been read correctly and I can push to the model
            if (runCondition) {
                subject_.setTime(lmtMaTime);
                subject_.setEmgs(emgData);
                subject_.setMuscleTendonLengths(lmtData);
                for (unsigned int i = 0; i < noDof_; ++i)
                    subject_.setMomentArms(momentArmsData.at(i), i);

                torqueErrorMinimizer_.setExternalTorques(externalTorqueData);
                torqueErrorMinimizer_.optimise();
                subject_.updateState();
                subject_.pushState();

#ifdef LOG_FILES
                //:TODO: Improve as now you are defining two times what you want to log
                vector<double> data;
                subject_.getActivations(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "Activations");
                subject_.getFiberLengths(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "FibreLengths");
                subject_.getNormFiberLengths(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "NormFibreLengths");
                subject_.getFiberVelocities(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "FibreVelocities");
                subject_.getNormFiberVelocities(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "NormFibreVelocities");
                subject_.getPennationAnglesAtT(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "PennationAngles");
                subject_.getMuscleForces(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "MuscleForces");
                subject_.getTorques(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "Torques");
				subject_.getTendonLengths(data);
				ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "TendonLengths");
				subject_.getNormTendonLengths(data);
				ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "NormTendonLengths");
                subject_.getEmgs(data);
                ModelEvaluationBase<Logger>::logger.log(emgTime-globalEmDelay_, data, "AdjustedEmgs");
                data.clear();
                for (unsigned i = 0; i < noDof_; ++i) {
                    vector<double> contribution;
                    subject_.getMusclesContribution(contribution, i);
                    data.insert(data.end(), contribution.begin(), contribution.end());
                }
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "MusclesContribution");
                data.clear();
                vector<double> components, weights;
                torqueErrorMinimizer_.getObjectiveFunctionComponentsAndWeightings(components, weights);
                data = components;
                data.insert(data.end(), weights.begin(), weights.end());
                ModelEvaluationBase<Logger>::logger.log(emgTime, data, "ObjectiveFunctionComponentsAndWeightings");
#endif

#ifdef LOG
                vector<double> adjustedEmg;
                subject_.getEmgs(adjustedEmg);
                cout << endl << endl << "EmgTime: " << emgTime << endl << "Adjusted EMG" << endl;
                for (auto& it : adjustedEmg)
                    cout << it << "\t";

                cout << endl << "LmtTime: " << lmtMaTime << endl << "Lmt" << endl;
                for (auto& it : lmtFrameFromQueue.value().data)
                    cout << it << "\t";

                for (unsigned int j = 0; j < dofNames_.size(); ++j) {
                    cout << endl << "MomentArms on: " << dofNames_.at(j) << endl;
                    for (auto& it : momentArmsFrameFromQueue.at(j).value().data)
                        cout << it << "\t";
                }
                cout << endl;
                cout << "External Torques: ";
                for (auto& it : externalTorquesFrameFromQueue.value().data)
                    cout << it << " ";
                cout << endl;

                vector<double> cTorques;
                subject_.getTorques(cTorques);
                for (unsigned int i = 0; i < cTorques.size(); ++i) {
                    cout << "Current Torque on " << dofNames_.at(i) << " ";
                    cout << cTorques.at(i) << endl;
                }
                cout << endl << "----------------------------------------" << endl;
#endif
            }

            //   float globalTimeLimit = ModelEvaluationBase<Logger>::getGlobalTimeLimit();
            //   runCondition = TimeCompare::less(emgTime,  globalTimeLimit) && TimeCompare::less(lmtMaTime, globalTimeLimit) && runCondition;
        } while (runCondition);

#ifdef LOG_FILES
        // Replaced sending a time of infinity to signify the end with closing the queue using the newer verion of Concurrency
        vector<string> queueNames{ "Activations", "FibreLengths", "NormFibreLengths", "FibreVelocities",
            "NormFibreVelocities", "PennationAngles", "MuscleForces", "Torques", "AdjustedEmgs",
            "MusclesContribution", "TendonLengths", "NormTendonLengths", "ObjectiveFunctionComponentsAndWeightings" };
        ModelEvaluationBase<Logger>::logger.close(queueNames);

#endif
        
        ModelEvaluationBase<Logger>::doneWithExecution();
        
#ifdef LOG
        cout << "Estimation completed." << endl;
#endif
    }
}