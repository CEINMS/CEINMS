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

#include <thread>
#include <chrono>
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include "ceinms/TimeCompare.h"
#include "ceinms/ModelEvaluationOffline.h"
#include "ceinms/ModelEvaluationBase.h"
//#include "SimpleFileLogger.h"
//#include "StorageLogger.h"

#define LOG_FILES
#define Evaluation_LOG

namespace ceinms {

    template<typename NMSmodelT, typename Logger>
    ModelEvaluationOffline<NMSmodelT, Logger>::ModelEvaluationOffline(
        EmgQueue& inputEmgQueue,
        LmtQueue& inputLmtQueue,
        ExternalTorquesQueue& inputExternalTorqueQueue,
        MultipleMomentArmsQueues& inputMomentArmsQueues,
        ScalarLogQueue& outputLogQueue,
		rtb::Concurrency::Latch& doneWithSubscriptions,
		rtb::Concurrency::Latch& doneWithExecution,
        NMSmodelT& subject,
        const vector<string>& valuesToLog) :
        ModelEvaluationBase<Logger>(
        inputEmgQueue,
        inputLmtQueue,
        inputExternalTorqueQueue,
        inputMomentArmsQueues,
        outputLogQueue,
        doneWithSubscriptions,
        doneWithExecution,
        valuesToLog),
        subject_(subject) {
        subject_.getDoFNames(dofNames_);
        noDof_ = dofNames_.size();
        globalEmDelay_ = subject.getGlobalEmDelay();
    }

    template<typename NMSmodelT, typename Logger>
    void ModelEvaluationOffline<NMSmodelT, Logger>::readDataFromQueues() {
        bool runCondition = true;
        //read EMG data
        do {
            auto frame(ModelEvaluationBase<Logger>::getEmgFromInputQueue());
            if (!frame.data.empty())
                emgFramesFromQueue_.push_back(frame);
            else runCondition = false;
        } while (runCondition);

        runCondition = true;
        //read LMT data
        do {
            auto frame(ModelEvaluationBase<Logger>::getLmtFromInputQueue());
            if (!frame.data.empty())
                lmtFramesFromQueue_.push_back(frame);
            else runCondition = false;
        } while (runCondition);

        runCondition = true;
        //read external torques
        do {
            auto frame(ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue());
            if (!frame.data.empty())
                externalTorquesFramesFromQueue_.push_back(frame);
            else runCondition = false;
        } while (runCondition);

        runCondition = true;
        //read moment arms data
        do {
            vector< MomentArmsFrame > maFrame;
            for (unsigned i = 0; i < noDof_; ++i) {
                auto currentMomentArms(ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(i));
                if (!currentMomentArms.data.empty())
                    maFrame.push_back(currentMomentArms);
                else runCondition = false;
            }
            if (runCondition)
                maFramesFromQueue_.push_back(maFrame);
        } while (runCondition);
    }

    template<typename NMSmodelT, typename Logger>
    void ModelEvaluationOffline<NMSmodelT, Logger>::initOfflineCurve() {
        bool runCondition(true);
        auto lmtIt(lmtFramesFromQueue_.begin());
        for (auto emgIt : emgFramesFromQueue_) {
            double emgTime = emgIt.time + globalEmDelay_;
            subject_.setTime(emgTime);  //this shouldn't affect anything, could be removed
            subject_.setEmgs(emgIt.data);
            if (lmtIt == lmtFramesFromQueue_.begin()) {
                subject_.updateActivations();
                subject_.pushState();
            }
            //we need to guard runCondition.. we cannot deferencing lmtIt->time if lmtIt == lmtDataFromQueue_.end(), so we have to split the check
            if (lmtIt == lmtFramesFromQueue_.end()) runCondition = false;
            if (runCondition) {
                if (TimeCompare::lessEqual(lmtIt->time, emgTime)) {
                    subject_.updateActivations();
                    subject_.setMuscleTendonLengths(lmtIt->data);
                    subject_.updateFibreLengths_OFFLINEPREP();
                    subject_.pushState();
                    ++lmtIt;
                }
            }
        }
        subject_.updateFibreLengthTraces();
    }

    template<typename NMSmodelT, typename Logger>
    void ModelEvaluationOffline<NMSmodelT, Logger>::operator()() {
        ModelEvaluationBase<Logger>::doneWithSubscriptions();
        vector<vector<double>> squaredDiffs;
#ifdef LOG
        cout << "starting consume" << endl;
#endif
        readDataFromQueues();
        initOfflineCurve();

        double externalTorqueTime = std::numeric_limits<double>::lowest();
        double emgTime = std::numeric_limits<double>::lowest();
        bool runCondition = !(emgFramesFromQueue_.empty() || maFramesFromQueue_.empty() || lmtFramesFromQueue_.empty());
        bool firstLmtArrived(false);

        while (runCondition) {  // while(runCondition)
            // 1. read lmt Data
            LmtFrame lmtFrame(lmtFramesFromQueue_.front());
            lmtFramesFromQueue_.pop_front();
            double lmtMaTime = lmtFrame.time;

            // 2. read moment arms data
            vector< MomentArmsFrame > momentArmsFrames(maFramesFromQueue_.front());
            maFramesFromQueue_.pop_front();

            // 3. read external Torque

            ExternalTorquesFrame externalTorquesFrame;
            while (TimeCompare::less(externalTorqueTime, lmtMaTime) && !externalTorquesFramesFromQueue_.empty()) {
                externalTorquesFrame = externalTorquesFramesFromQueue_.front();
                externalTorquesFramesFromQueue_.pop_front();
                externalTorqueTime = externalTorquesFrame.time;
            }

            //note: check ModelEvaluationOnline for info about firstLmtArrived

            // 4. read emgs
            EmgFrame emgFrame;
            while (TimeCompare::less(emgTime, lmtMaTime)) {
                emgFrame = emgFramesFromQueue_.front();
                emgFramesFromQueue_.pop_front();
                emgTime = emgFrame.time + globalEmDelay_;
                runCondition = runCondition && !emgFrame.data.empty();
                if (!TimeCompare::less(emgTime, lmtMaTime)) firstLmtArrived = true;
                if (!firstLmtArrived && runCondition) {
                    subject_.setTime(emgTime);
                    subject_.setEmgs(emgFrame.data);
                    subject_.updateActivations();
                    subject_.pushState();
                }
            }

            subject_.setTime(emgTime);
            subject_.setEmgs(emgFrame.data);
            subject_.setMuscleTendonLengths(lmtFrame.data);
            for (unsigned int i = 0; i < noDof_; ++i)
                subject_.setMomentArms(momentArmsFrames.at(i).data, i);
            subject_.updateState_OFFLINE();
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
            subject_.getMuscleForcesPassive(data);
            ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "MuscleForcesPassive");
            subject_.getMuscleForcesActive(data);
            ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "MuscleForcesActive");
            subject_.getMuscleForcesDamping(data);
            ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "MuscleForcesDamping");
			subject_.getTendonLengths(data);
			ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "TendonLengths");
			subject_.getNormTendonLengths(data);
			ModelEvaluationBase<Logger>::logger.log(lmtMaTime, data, "NormTendonLengths");
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
            cout << "modelEval time: " << lmtMaTime << endl;
            for (unsigned i(0); i < 100; ++i) cout << ".";
            cout << endl;

#ifdef Evaluation_LOG
            cout << endl << endl << "EmgTime: " << emgTime << endl << "EMG" << endl;
            for (auto it : emgFrame.data)
                cout << it << "\t";

            cout << endl << "Activations: " << endl;
            vector<double> cActivations;
            subject_.getActivations(cActivations);
            for (auto it : cActivations)
                cout << it << "\t";

            cout << endl << "LmtTime: " << lmtMaTime << endl << "Lmt" << endl;
            for (auto it : lmtFrame.data)
                cout << it << "\t";

            vector<double> cMuscleForces;
            subject_.getMuscleForces(cMuscleForces);
            cout << endl << "Muscle forces: " << endl;
            for (auto it : cMuscleForces)
                cout << it << "\t";

            for (unsigned int j = 0; j < dofNames_.size(); ++j) {
                cout << endl << "MomentArms on: " << dofNames_.at(j) << endl;
                for (auto it : momentArmsFrames.at(j).data)
                    cout << it << "\t";
            }

            cout << endl << "External Torques: ";
            for (auto it : externalTorquesFrame.data)
                cout << it << " ";

            vector<double> cTorques;
            subject_.getTorques(cTorques);
            for (unsigned int i = 0; i < cTorques.size(); ++i) {
                cout << "\nCurrent Torque on " << dofNames_.at(i) << " ";
                cout << cTorques.at(i);
            }
            cout << endl << "----------------------------------------" << endl;
            vector<double> currentSquaredDiff;
            std::transform(
                std::begin(externalTorquesFrame.data),
                std::end(externalTorquesFrame.data),
                std::begin(cTorques),
                std::back_inserter(currentSquaredDiff), [](double first, double second){
                auto v =  (first - second)*(first - second); 
                cout << "Squared diffs: " << v << endl;
                return v; });
            squaredDiffs.emplace_back(currentSquaredDiff);
#endif
            runCondition = runCondition = !(emgFramesFromQueue_.empty() || maFramesFromQueue_.empty() || lmtFramesFromQueue_.empty());
        } //end while(runCondition)

#ifdef LOG_FILES
        // Replaced sending a time of infinity to signify the end with closing the queue using the newer verion of Concurrency
        vector<string> queueNames{ "Activations", "FibreLengths", "NormFibreLengths", "FibreVelocities",
            "NormFibreVelocities", "PennationAngles", "MuscleForces", "MuscleForcesPassive", "MuscleForcesActive", "MuscleForcesDamping",
            "TendonLengths", "NormTendonLengths", "Torques", "MusclesContribution" };
        ModelEvaluationBase<Logger>::logger.close(queueNames);

#endif

        ModelEvaluationBase<Logger>::doneWithExecution();
#ifdef Evaluation_LOG
        cout << "Joint moments RMSE\n";
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

    template<typename NMSmodelT, typename Logger>
    ModelEvaluationOffline<NMSmodelT, Logger>::~ModelEvaluationOffline() { }
}