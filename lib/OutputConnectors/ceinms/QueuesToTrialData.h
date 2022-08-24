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

#ifndef ceinms_QueuesToTrialData_h
#define ceinms_QueuesToTrialData_h

#include <iostream>
#include <fstream>
#include <filesystem>

#include "rtb/concurrency/Queue.h"
#include "rtb/concurrency/Latch.h"

#include "ceinms/TrialData.h"
#include "ceinms/queues/EmgQueue.h"
#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/ExternalTorquesQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"

namespace ceinms {

    class QueuesToTrialData {
    public:
        QueuesToTrialData() = delete;
        QueuesToTrialData(const QueuesToTrialData&) = delete;
        QueuesToTrialData& operator=(const QueuesToTrialData&) = delete;

        template<typename NMSmodel>
        QueuesToTrialData(
            EmgQueue& inputEmgQueue,
            LmtQueue& inputLmtQueue,
            ExternalTorquesQueue& inputExternalTorqueQueue,
            MultipleMomentArmsQueues& inputMomentArmsQueues,
			rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
            NMSmodel& subject,
            std::string id) :
            inputEmgQueue_(inputEmgQueue),
            inputLmtQueue_(inputLmtQueue),
            inputExternalTorqueQueue_(inputExternalTorqueQueue),
            inputMomentArmsQueues_(inputMomentArmsQueues),
            doneWithSubscriptions_(doneWithSubscriptions),
            doneWithExecution_(doneWithExecution) {
           
            subject.getMuscleNames(muscleNames_);
            subject.getDoFNames(dofNames_);
            subject.getMuscleNamesOnDofs(maMuscleNames_);
            data_.setProperties(dofNames_, muscleNames_, id);
        };
        ~QueuesToTrialData() = default;
        void operator()();
        TrialData getTrialData();

    private:
        TrialData data_;
        EmgQueue& inputEmgQueue_;
        LmtQueue& inputLmtQueue_;
        ExternalTorquesQueue& inputExternalTorqueQueue_;
        MultipleMomentArmsQueues& inputMomentArmsQueues_;
		rtb::Concurrency::Latch& doneWithSubscriptions_;
		rtb::Concurrency::Latch& doneWithExecution_;
        std::vector<std::string> muscleNames_, dofNames_;
        std::vector<std::vector<std::string>> maMuscleNames_;
    };
}

#endif
