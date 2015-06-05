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

#ifndef ceinms_ExternalTorquesFromX_h
#define ceinms_ExternalTorquesFromX_h

#include <vector>
#include <string>
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/queues/ScalarVectorQueue.h"

namespace ceinms {
    class ExternalTorquesFromX
    {
    public:
        using TorqueFrame = ScalarVectorFrame;
        using TorqueData = TorqueFrame::type;
        template <typename NMSModelT>
        ExternalTorquesFromX(
            ScalarVectorQueue& outputExternalTorquesQueue,
            Concurrency::Latch& doneWithSubscriptions,
            Concurrency::Latch& doneWithExecution,
            const NMSModelT& subject);

        virtual ~ExternalTorquesFromX();
        virtual void operator()() = 0;
        void updateExternalTorques(const TorqueData& currentExternalTorquesData, double currentTime);
        void doneWithSubscriptions();
        void doneWithExecution();
        void sendEndOfData();

    protected:
        std::vector< std::string > dofNames_;
    private:
        ScalarVectorQueue& outputExternalTorquesQueue_;
        Concurrency::Latch& doneWithSubscriptions_;
        Concurrency::Latch& doneWithExecution_;
    };

    template <typename NMSModelT>
    ExternalTorquesFromX::ExternalTorquesFromX(
        ScalarVectorQueue& outputExternalTorquesQueue,
        Concurrency::Latch& doneWithSubscriptions,
        Concurrency::Latch& doneWithExecution,
        const NMSModelT& subject) :
        outputExternalTorquesQueue_(outputExternalTorquesQueue),
        doneWithSubscriptions_(doneWithSubscriptions),
        doneWithExecution_(doneWithExecution)
    {
        subject.getDoFNames(dofNames_);
    }
}

#endif
