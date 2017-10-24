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
 * Author(s): C. Pizzolato, M. Reggiani, E. Ceseracciu                        *
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

#include "ceinms/QueuesToTrialData.h"
#include "ceinms/EndOfData.h"

namespace ceinms {
    void QueuesToTrialData::operator()() {

        inputEmgQueue_.subscribe();
        inputLmtQueue_.subscribe();
        inputExternalTorqueQueue_.subscribe();
        for (auto& it : inputMomentArmsQueues_)
            (*it).subscribe();

        doneWithSubscriptions_.wait();
        doneWithExecution_.wait(); //wait until all the data has been produced and put in the queues

        double externalTorqueTime = std::numeric_limits<double>::lowest();
        bool lmtRunning = true;
        bool torqueRunning = true;
        bool emgRunning = true;

        DataTable<double> lmtData, torqueData, emgData;
        std::vector<DataTable<double>> maData;
        
        emgData.setLabels(muscleNames_);
        lmtData.setLabels(muscleNames_);
        maData.resize(dofNames_.size());
        for (auto i(0); i < dofNames_.size(); ++i)
            maData.at(i).setLabels(maMuscleNames_.at(i));
        torqueData.setLabels(dofNames_);
        
        do {  // while(runCondition)

            // 1. read lmt Data
            if (lmtRunning) {
                ScalarVectorFrame lmtFrameFromQueue = inputLmtQueue_.pop();
                if (!EndOfData::isEod(lmtFrameFromQueue)) {
                    lmtData.pushRow(lmtFrameFromQueue.time, lmtFrameFromQueue.data);
                    // 2. read moment arms data
                    for (unsigned int i = 0; i < inputMomentArmsQueues_.size(); ++i) {
                        MomentArmsFrame momentArmsFrameFromQueue = (*inputMomentArmsQueues_.at(i)).pop();
                        maData.at(i).pushRow(momentArmsFrameFromQueue.time, momentArmsFrameFromQueue.data);
                    }
                }
                else
                    lmtRunning = false;
            }

            //if there is no external torque, ExternalTorqueFromX is configured to just send an EOD frame
            //in that case, we'll get the EOD here.. the result will be a
            // 3. read external Torque
            if (torqueRunning) {
                ScalarVectorFrame externalTorquesFrameFromQueue = inputExternalTorqueQueue_.pop();
                if (!EndOfData::isEod(externalTorquesFrameFromQueue)) {
                    torqueData.pushRow(externalTorquesFrameFromQueue.time, externalTorquesFrameFromQueue.data);
                }
                else
                    torqueRunning = false;
            }

            if (emgRunning) {
                ScalarVectorFrame emgFrameFromQueue = inputEmgQueue_.pop();
                if (!EndOfData::isEod(emgFrameFromQueue)) {
                    emgData.pushRow(emgFrameFromQueue.time, emgFrameFromQueue.data);
                }
                else
                    emgRunning = false;
            }
        } while (lmtRunning || emgRunning || torqueRunning);

        data_.setEmgData(emgData);

        double startTime = std::max(lmtData.getStartTime(), torqueData.getStartTime());
        double finalTime = std::min(lmtData.getFinalTime(), torqueData.getFinalTime());
        lmtData.cropToTime(startTime, finalTime);
        for (auto& m : maData) {
            m.cropToTime(startTime, finalTime);
        }
        torqueData.cropToTime(startTime, finalTime);
        data_.setLmtMaData(lmtData, maData);
        data_.setTorqueData(torqueData);
    }

    TrialData QueuesToTrialData::getTrialData()
    {
        return data_;
    }
}