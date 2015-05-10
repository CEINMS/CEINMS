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

#include "SimpleFileLogger.h"
#include "StorageLogger.h"
#include "ceinms/TimeCompare.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include "ModelEvaluationOffline.h"
#include "ModelEvaluationBase.h"
#include "ceinms/InputConnectors.h"
#include "ceinms/OutputConnectors.h"
#define LOG_FILES
#define LOG

namespace ceinms {
    template <typename NMSmodelT, typename Logger>
    ModelEvaluationOffline<NMSmodelT, Logger>::ModelEvaluationOffline(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, NMSmodelT& subject, const vector<string>& valuesToLog)
        :ModelEvaluationBase<Logger>::ModelEvaluationBase(inputConnectors, outputConnectors, valuesToLog), subject_(subject)
    {
        subject_.getDoFNames(dofNames_);
        noDof_ = dofNames_.size();
        globalEmDelay_ = subject.getGlobalEmDelay();
    }


    template <typename NMSmodelT, typename Logger>
    void ModelEvaluationOffline<NMSmodelT, Logger>::readDataFromQueues() {

        bool runCondition = true;
        //read EMG data
        do{
            auto frame(ModelEvaluationBase<Logger>::getEmgFromInputQueue());
            if (!frame.data.empty())
                emgDataFromQueue_.push_back(frame);
            else runCondition = false;
        } while (runCondition);

        runCondition = true;
        //read LMT data
        do{
            auto frame(ModelEvaluationBase<Logger>::getLmtFromInputQueue());
            if (!frame.data.empty())
                lmtDataFromQueue_.push_back(frame);
            else runCondition = false;
        } while (runCondition);


        runCondition = ModelEvaluationBase<Logger>::externalTorquesAvailable();
        //read external torques
        do{
            auto frame(ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue());
            if (!frame.data.empty())
                externalTorquesDataFromQueue_.push_back(frame);
            else runCondition = false;
        } while (runCondition);


        runCondition = true;
        //read moment arms data
        do{
            vector< InputConnectors::FrameType > maFrame;
            for (unsigned i = 0; i < noDof_; ++i) {
                auto currentMomentArms(ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(i));
                if (!currentMomentArms.data.empty())
                    maFrame.push_back(currentMomentArms);
                else runCondition = false;
            }
            if (runCondition)
                maDataFromQueue_.push_back(maFrame);
        } while (runCondition);

    }

    template <typename NMSmodelT, typename Logger>
    void ModelEvaluationOffline<NMSmodelT, Logger>::initOfflineCurve() {

        auto lmtIt(lmtDataFromQueue_.begin());
        for (auto emgIt : emgDataFromQueue_) {
            double emgTime = emgIt.time + globalEmDelay_;
            subject_.setTime(emgTime);
            subject_.setEmgs(emgIt.data);
            if (lmtIt == lmtDataFromQueue_.begin()) subject_.updateActivations();
            if (lmtIt != lmtDataFromQueue_.end() && TimeCompare::lessEqual(lmtIt->time, emgTime)) {
                subject_.updateActivations();
                subject_.setMuscleTendonLengths(lmtIt->data);
                subject_.updateFibreLengths_OFFLINEPREP();
                ++lmtIt;
            }
            subject_.pushState();
        }
        subject_.updateFibreLengthTraces();
    }


    template <typename NMSmodelT, typename Logger>
    void ModelEvaluationOffline<NMSmodelT, Logger>::operator()() {

        ModelEvaluationBase<Logger>::subscribeToInputConnectors();

#ifdef LOG
        cout << "starting consume" << endl;
#endif
        readDataFromQueues();
        initOfflineCurve();

        double externalTorqueTime = std::numeric_limits<double>::lowest();
        double emgTime = std::numeric_limits<double>::lowest();
        bool runCondition = !(emgDataFromQueue_.empty() || maDataFromQueue_.empty() || lmtDataFromQueue_.empty());
        bool firstLmtArrived(false);

        while (runCondition) {  // while(runCondition)

            // 1. read lmt Data
            InputConnectors::FrameType lmtFrameFromQueue(lmtDataFromQueue_.front());
            lmtDataFromQueue_.pop_front();
            double lmtMaTime = lmtFrameFromQueue.time;

            // 2. read moment arms data
            vector< InputConnectors::FrameType > momentArmsFrameFromQueue(maDataFromQueue_.front());
            maDataFromQueue_.pop_front();

            // 3. read external Torque
            InputConnectors::FrameType externalTorquesFrameFromQueue;
            while (TimeCompare::less(externalTorqueTime, lmtMaTime) && !externalTorquesDataFromQueue_.empty()) {
                externalTorquesFrameFromQueue = externalTorquesDataFromQueue_.front();
                externalTorquesDataFromQueue_.pop_front();
                externalTorqueTime = externalTorquesFrameFromQueue.time;
            }

            // 4. read emgs
            InputConnectors::FrameType emgFrameFromQueue;
            while (TimeCompare::less(emgTime, lmtMaTime)) {
                emgFrameFromQueue = emgDataFromQueue_.front();
                emgDataFromQueue_.pop_front();
                emgTime = emgFrameFromQueue.time + globalEmDelay_;
                runCondition = runCondition && !emgFrameFromQueue.data.empty();
                if (!TimeCompare::less(emgTime, lmtMaTime)) firstLmtArrived = true;
                if (!firstLmtArrived && runCondition) {
                    subject_.setTime(emgTime);
                    subject_.setEmgs(emgFrameFromQueue.data);
                    subject_.updateActivations();
                    subject_.pushState();
                }
            }

            subject_.setTime(emgTime);
            subject_.setEmgs(emgFrameFromQueue.data);
            subject_.setMuscleTendonLengths(lmtFrameFromQueue.data);
            for (unsigned int i = 0; i < noDof_; ++i)
                subject_.setMomentArms(momentArmsFrameFromQueue.at(i).data, i);
            subject_.updateState_OFFLINE();
            subject_.pushState();
#ifdef LOG_FILES
            //:TODO: Improve as now you are defining two times what you want to log
            vector<double> data;
            subject_.getActivations(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "Activations");
            subject_.getFiberLengths(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "FiberLenghts");
            subject_.getNormFiberLengths(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "NormFiberLengths");
            subject_.getFiberVelocities(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "FiberVelocities");
            subject_.getNormFiberVelocities(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "NormFiberVelocities");
            subject_.getPennationAnglesAtT(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "PennationAngles");
            subject_.getMuscleForces(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "MuscleForces");
            subject_.getTorques(data);
            ModelEvaluationBase<Logger>::logger.log(emgTime, data, "Torques");
#endif

#ifdef LOG
            cout << endl << endl << "EmgTime: " << emgTime << endl << "EMG" << endl;
            for (auto& it : emgFrameFromQueue.data)
                cout << it << "\t";

            cout << endl << "Activations: " << endl;
            vector<double> cActivations;
            subject_.getActivations(cActivations);
            for (auto& it : cActivations)
                cout << it << "\t";

            cout << endl << "LmtTime: " << lmtMaTime << endl << "Lmt" << endl;
            for (auto& it : lmtFrameFromQueue.data)
                cout << it << "\t";

            vector<double> cMuscleForces;
            subject_.getMuscleForces(cMuscleForces);
            cout << endl << "Muscle forces: " << endl;
            for (auto& it : cMuscleForces)
                cout << it << "\t";

            for (unsigned int j = 0; j < dofNames_.size(); ++j) {
                cout << endl << "MomentArms on: " << dofNames_.at(j) << endl;
                for (auto& it : momentArmsFrameFromQueue.at(j).data)
                    cout << it << "\t";
            }

            cout << endl << "External Torques: ";
            for (auto& it : externalTorquesFrameFromQueue.data)
                cout << it << " ";

            vector<double> cTorques;
            subject_.getTorques(cTorques);
            for (unsigned int i = 0; i < cTorques.size(); ++i) {
                cout << "\nCurrent Torque on " << dofNames_.at(i) << " ";
                cout << cTorques.at(i);
            }
            cout << endl << "----------------------------------------" << endl;
#endif
            runCondition = runCondition = !(emgDataFromQueue_.empty() || maDataFromQueue_.empty() || lmtDataFromQueue_.empty());
        } //end while(runCondition)



#ifdef LOG_FILES
        //:TODO: improve this... This is the end... so I'm just writing on the queue a time equal to 0
        vector<double> endData;
        double endTime = std::numeric_limits<double>::infinity();
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Activations");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FiberLenghts");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "NormFiberLengths");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FiberVelocities");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "NormFiberVelocities");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "PennationAngles");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "MuscleForces");
        ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Torques");
#endif

        this->doneWithExecution();
#ifdef LOG
        cout << "Estimation completed. " << endl;
#endif

    }


    template <typename NMSmodelT, typename Logger>
    ModelEvaluationOffline<NMSmodelT, Logger>::~ModelEvaluationOffline()
    {

    }


}
