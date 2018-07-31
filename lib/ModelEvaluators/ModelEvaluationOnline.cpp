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
#include <boost/concept_check.hpp>
using std::string;

#include "ceinms/TimeCompare.h"
#include "ceinms/EndOfData.h"

#define LOG_FILES
//#define LOG

namespace ceinms {
    template <typename NMSmodelT, typename Logger>
    ModelEvaluationOnline<NMSmodelT, Logger>::ModelEvaluationOnline(
        EmgQueue& inputEmgQueue,
        LmtQueue& inputLmtQueue,
        ExternalTorquesQueue& inputExternalTorqueQueue,
        MultipleMomentArmsQueues& inputMomentArmsQueues,
        ScalarLogQueue& outputLogQueue,
        Concurrency::Latch& doneWithSubscriptions,
        Concurrency::Latch& doneWithExecution,
        NMSmodelT& subject,
        const vector<string>& valuesToLog,
        bool showLog)
        :ModelEvaluationBase<Logger>(
        inputEmgQueue,
        inputLmtQueue,
        inputExternalTorqueQueue,
        inputMomentArmsQueues,
        outputLogQueue,
        doneWithSubscriptions,
        doneWithExecution,
        valuesToLog,
        showLog),
        subject_(subject)
    {
        subject_.getDoFNames(dofNames_);
        noDof_ = dofNames_.size();
        globalEmDelay_ = subject.getGlobalEmDelay();
    }

    template <typename NMSmodelT, typename Logger>
    void ModelEvaluationOnline<NMSmodelT, Logger>::operator()() {


#ifdef LOG
        cout << "starting consume" << endl;
#endif

        vector<vector<double>> squaredDiffs;
        double externalTorqueTime = std::numeric_limits<double>::lowest();
        double emgTime = std::numeric_limits<double>::lowest();
        bool runCondition(true);
        bool firstLmtArrived(false);
        bool externalTorquesRunCondition(true); //we first assume there are ext forces
        cout << subject_ << endl;

        ModelEvaluationBase<Logger>::doneWithSubscriptions();
        do {  // while(runCondition)

            // 1. read lmt Data
            LmtFrame lmtFrameFromQueue;
            ModelEvaluationBase<Logger>::getLmtFromInputQueue(lmtFrameFromQueue);
            double lmtMaTime = lmtFrameFromQueue.time;

            // update runCondition
            runCondition = runCondition && !lmtFrameFromQueue.data.empty();

            // 2. read moment arms data
            vector< MomentArmsFrame > momentArmsFramesFromQueue;
            momentArmsFramesFromQueue.resize(noDof_);
            for (unsigned int i = 0; i < noDof_; ++i) {
                ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue((momentArmsFramesFromQueue.at(i)), i);
                runCondition = runCondition && !momentArmsFramesFromQueue.at(i).data.empty();
            }

            //if no external torques are available, the producer will send an EndOfData frame
            // 3. read external Torque
            ExternalTorquesFrame externalTorquesFrameFromQueue;
            externalTorquesFrameFromQueue.data.resize(noDof_);
            if (externalTorquesRunCondition) {
                while (TimeCompare::less(externalTorqueTime, lmtMaTime)) {
                    ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(externalTorquesFrameFromQueue);
                    if (EndOfData::isEod(externalTorquesFrameFromQueue))
                        externalTorquesRunCondition = false;
                    externalTorqueTime = externalTorquesFrameFromQueue.time;
                }
            }

            /* Note about firstLmtArrived: all the emgs before the first frame of lmt are
            ** read, while after the first frame of lmt just the correspondant emgs are used.
            ** This is for a few different reasons:
            ** (i)   in the EMG-assisted mode we can work only with emgs that correspond to a lmt frame.
            **       Also, different modes should behave in the same way
            ** (ii)  Since the execution of the model behave in this way, we have to calibrate consistently
            **       to avoid problems with parameters c1 and c2 in the activation model
            ** (iii) EMG are supposed to be sampled at a higher frequency than lmt, also undersampling
            **       emgs doesn't chenage the information content of the signal, since we are working
            **       with linear envelopes. Moreover, we save some time if we undersample emgs
            ** (iv)  Since the activation model uses a IIR filter, we prefer to 'charge' the filter
            **       with previous emgs, instead of starting a transient from 0.
            ** All of the above is subject to change, until we find a smarter way to do things
            */
            // 4. read emgs
            EmgFrame emgFrameFromQueue;
            emgFrameFromQueue.data.resize(subject_.getNoMuscles());
            while (TimeCompare::less(emgTime, lmtMaTime) && runCondition) {
                ModelEvaluationBase<Logger>::getEmgFromInputQueue(emgFrameFromQueue);
                emgTime = emgFrameFromQueue.time + globalEmDelay_;
                runCondition = runCondition && !emgFrameFromQueue.data.empty();
                if (!TimeCompare::less(emgTime, lmtMaTime)) firstLmtArrived = true;
                if (runCondition && !firstLmtArrived) {
                    subject_.setTime(emgTime); //this shouldt affect anything anyway
                    subject_.setEmgs(emgFrameFromQueue.data);
                    subject_.updateActivations();
                    subject_.pushState();
                }
            }

            //5. lmt, ma, emg, extTorques have been read correctly and I can push to the model
            if (runCondition) {
                subject_.setTime(lmtMaTime);
                subject_.setEmgs(emgFrameFromQueue.data);
                subject_.setMuscleTendonLengths(lmtFrameFromQueue.data);
                for (unsigned int i = 0; i < noDof_; ++i)
                    subject_.setMomentArms(momentArmsFramesFromQueue.at(i).data, i);
                subject_.updateState();
                subject_.pushState();
#ifdef LOG_FILES
                //:TODO: Improve as now you are defining two times what you want to log
                vector<double> data;
                subject_.getActivations(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "Activations");
                subject_.getFiberLengths(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "FibreLengths");
                subject_.getNormFiberLengths(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "NormFibreLengths");
                subject_.getFiberVelocities(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "FibreVelocities");
                subject_.getNormFiberVelocities(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "NormFibreVelocities");
                subject_.getPennationAnglesAtT(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "PennationAngles");
                subject_.getMuscleForces(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "MuscleForces");
                subject_.getTorques(data);
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "Torques");
                data.clear();
                for (unsigned i = 0; i < noDof_; ++i) {
                    vector<double> contribution;
                    subject_.getMusclesContribution(contribution, i);
                    data.insert(data.end(), contribution.begin(), contribution.end());
                }
                ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "MusclesContribution");
#endif


#ifdef LOG
                if (this->showLog_) {
                    cout << endl << endl << "EmgTime: " << emgTime << endl << "EMG" << endl;
                    for (auto& it : emgFrameFromQueue.data)
                        cout << it << "\t";

                    cout << endl << "LmtTime: " << lmtMaTime << endl << "Lmt" << endl;
                    for (auto& it : lmtFrameFromQueue.data)
                        cout << it << "\t";

                    for (unsigned int j = 0; j < dofNames_.size(); ++j) {
                        cout << endl << "MomentArms on: " << dofNames_.at(j) << endl;
                        for (auto& it : momentArmsFramesFromQueue.at(j).data)
                            cout << it << "\t";
                    }

                    if (externalTorquesRunCondition) {
                        for (auto& it : externalTorquesFrameFromQueue.data)
                            cout << it << " ";
                    }

                    vector<double> cTorques;
                    subject_.getTorques(cTorques);
                    for (unsigned int i = 0; i < cTorques.size(); ++i) {
                        cout << "\nCurrent Torque on " << dofNames_.at(i) << " ";
                        cout << cTorques.at(i);
                    }
                    cout << endl << "----------------------------------------" << endl;
                    
                    vector<double> currentSquaredDiff;
                    std::transform(
                        std::begin(externalTorquesFrameFromQueue.data),
                        std::end(externalTorquesFrameFromQueue.data),
                        std::begin(cTorques),
                        std::back_inserter(currentSquaredDiff), [](double first, double second){
                        return (first - second)*(first - second); });
                   squaredDiffs.emplace_back(currentSquaredDiff);

                }
#endif
            }


            //   float globalTimeLimit = ModelEvaluationBase<Logger>::getGlobalTimeLimit();
            //   runCondition = TimeCompare::less(emgTime,  globalTimeLimit) && TimeCompare::less(lmtMaTime, globalTimeLimit) && runCondition;
        } while (runCondition);

#ifdef LOG_FILES
        //:TODO: improve this... This is the end... so I'm just writing on the queue a time equal to 0
        vector<double> endData;
        double endTime = std::numeric_limits<double>::infinity();
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Activations");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FibreLengths");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "NormFibreLengths");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FibreVelocities");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "NormFibreVelocities");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "PennationAngles");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "MuscleForces");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Torques");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "MusclesContribution");
#endif

        ModelEvaluationBase<Logger>::doneWithExecution();

#ifdef LOG
        std::cout << "Joint moments RMSE\n";
        vector<double> rmse(squaredDiffs.front().size(), 0);

        for (auto currentSquaredDiff : squaredDiffs) {
            for (int i(0); i < currentSquaredDiff.size(); ++i)
                rmse.at(i) += currentSquaredDiff.at(i);
        }
        for (int i(0); i < dofNames_.size(); ++i)
            cout << dofNames_.at(i) << " " << sqrt(rmse.at(i) / squaredDiffs.size()) << " Nm" << endl;
        cout << "Estimation completed. " << endl;
        
#endif
    }
}