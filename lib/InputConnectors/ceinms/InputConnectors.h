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

#ifndef ceinms_InputConnectors_h
#define ceinms_InputConnectors_h

#include "rtb/concurrency/Queue.h"
#include "rtb/concurrency/Latch.h"
#include "ceinms/QueueData.h"

#include <list>
#include <vector>

namespace ceinms {
    class InputConnectors {
    public:
        InputConnectors();
        virtual ~InputConnectors();
        InputConnectors(const InputConnectors&) = delete;
        typedef QueueData< std::vector< double > > FrameType;
        rtb::Concurrency::Queue< FrameType > queueEmg;
		rtb::Concurrency::Queue< FrameType > queueLmt;
        std::vector< rtb::Concurrency::Queue< FrameType >* > queueMomentArms;
		rtb::Concurrency::Queue< FrameType >  queueExternalTorques;
		rtb::Concurrency::Latch doneWithSubscription;

        double globalTimeLimit;
        bool externalTorquesAvailable;
    };
}

#endif
