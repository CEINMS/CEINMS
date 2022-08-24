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

#ifndef ceinms_LmtMaFromX_h
#define ceinms_LmtMaFromX_h

#include <vector>
#include <string>
#include <memory>

#include "rtb/concurrency/Concurrency.h"

#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"

namespace ceinms {

    class LmtMaFromX {
    public:
        template<typename NMSModelT>
        LmtMaFromX(
            LmtQueue& outputLmtQueue,
            MultipleMomentArmsQueues& outputMomentArmsQueue,
			rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
            const NMSModelT& subject);
        virtual ~LmtMaFromX();
        virtual void operator()() = 0;
        void updateLmt(const LmtData& currentLmtData, double currentTime);
        void updateMomentArms(const MomentArmsData& currentMomentArmsData, double currentTime, unsigned int whichDof);
        void doneWithSubscriptions();
        void doneWithExecution();
        void sendLmtEndOfData();
        void sendMaEndOfData();

    protected:
        std::vector<std::string> musclesNames_;
        std::vector<std::string> dofNames_;
        std::vector<std::vector<std::string>> muscleNamesOnDofs_;

    private:
        void initialiseMomentArmsQueue();
        LmtQueue& outputLmtQueue_;
        MultipleMomentArmsQueues& outputMomentArmsQueue_;
        rtb::Concurrency::Latch& doneWithSubscriptions_;
        rtb::Concurrency::Latch& doneWithExecution_;
    };

    template<typename NMSModelT>
    LmtMaFromX::LmtMaFromX(
        LmtQueue& outputLmtQueue,
        MultipleMomentArmsQueues& outputMomentArmsQueue,
        rtb::Concurrency::Latch& doneWithSubscriptions,
        rtb::Concurrency::Latch& doneWithExecution,
        const NMSModelT& subject) :
        outputLmtQueue_(outputLmtQueue),
        outputMomentArmsQueue_(outputMomentArmsQueue),
        doneWithSubscriptions_(doneWithSubscriptions),
        doneWithExecution_(doneWithExecution) {

        subject.getMuscleNames(musclesNames_);
        subject.getDoFNames(dofNames_);
        subject.getMuscleNamesOnDofs(muscleNamesOnDofs_);
        initialiseMomentArmsQueue();
    }
}

#endif
