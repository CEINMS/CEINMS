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

#include "QueuesToTrialData.h"
#include "InputConnectors.h"

namespace ceinms {
    void QueuesToTrialData::operator()() {

        inputConnectors_.queueLmt.subscribe();
        inputConnectors_.queueEmg.subscribe();
        for (auto& it : inputConnectors_.queueMomentArms)
            (*it).subscribe();
        inputConnectors_.queueExternalTorques.subscribe();
        inputConnectors_.doneWithSubscription.wait();
        outputConnectors_.doneWithExecution.wait();

        if (!inputConnectors_.externalTorquesAvailable)
            return;
        double externalTorqueTime = std::numeric_limits<double>::lowest();
        bool lmtRunning = true;
        bool torqueRunning = true;
        bool emgRunning = true;
        do {  // while(runCondition)

            // 1. read lmt Data
            if (lmtRunning) {
                InputConnectors::FrameType lmtFrameFromQueue = inputConnectors_.queueLmt.pop();
                if (lmtFrameFromQueue.time < std::numeric_limits<double>::infinity()) {
                    data_.lmtData.pushRow(lmtFrameFromQueue.time, lmtFrameFromQueue.data);
                    // 2. read moment arms data
                    for (unsigned int i = 0; i < inputConnectors_.queueMomentArms.size(); ++i) {
                        InputConnectors::FrameType momentArmsFrameFromQueue = (*inputConnectors_.queueMomentArms.at(i)).pop();
                        data_.maData.at(i).pushRow(momentArmsFrameFromQueue.time, momentArmsFrameFromQueue.data);
                    }
                }
                else
                    lmtRunning = false;
            }

            // 3. read external Torque
            if (torqueRunning) {
                InputConnectors::FrameType externalTorquesFrameFromQueue = inputConnectors_.queueExternalTorques.pop();
                if (externalTorquesFrameFromQueue.time < std::numeric_limits<double>::infinity()) {
                    data_.torqueData.pushRow(externalTorquesFrameFromQueue.time, externalTorquesFrameFromQueue.data);
                }
                else
                    torqueRunning = false;
            }

            if (emgRunning) {
                InputConnectors::FrameType emgFrameFromQueue = inputConnectors_.queueEmg.pop();
                if (emgFrameFromQueue.time < std::numeric_limits<double>::infinity()) {
                    data_.emgData.pushRow(emgFrameFromQueue.time, emgFrameFromQueue.data);
                }
                else
                    emgRunning = false;
            }

        } while (lmtRunning || emgRunning || torqueRunning);
    }

    TrialData QueuesToTrialData::getTrialData()
    {
        return data_;
    }
}
