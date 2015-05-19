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
* Author(s): C. Pizzolato                                                    *
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

#ifndef ceinms_QueuesToJointReactionForces_h
#define ceinms_QueuesToJointReactionForces_h
#include "ceinms/QueuesToXForces.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/queues/ContactForcesQueue.h"
#include "ceinms/queues/EstimatedTorquesQueue.h"
#include "ceinms/queues/GeneralisedCoordinatesQueue.h"
#include "ceinms/queues/MuscleForcesQueue.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"

namespace ceinms {

    class QueuesToJointReactionForces : public QueuesToXForces {

    public:

        template <typename NMSmodelT>
        QueuesToJointReactionForces(
            GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue,
            MultipleExternalForcesQueue& inputExternalForcesQueue,
            MuscleForcesQueue& inputMuscleForcesQueue,
            EstimatedTorquesQueue& inputEstimatedTorquesQueue,
            ContactForcesQueue& outputJointReactionForcesQueue,
            Concurrency::Latch& latch,
            const NMSmodelT& subject,
            const std::string& osimModelFilename,
            const std::string& externalLoadsXmlFilename,
            const std::vector< std::string >& jointsToLog);
 
        virtual void operator()() override;
        virtual ~QueuesToJointReactionForces() override {}

    private:
        GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue_;
        MuscleForcesQueue& inputMuscleForcesQueue_;
        EstimatedTorquesQueue& inputEstimatedTorquesQueue_;
        ContactForcesQueue& outputJointReactionForcesQueue_;
    
    };


    template <typename NMSmodelT>
    QueuesToJointReactionForces::QueuesToJointReactionForces(
        GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue,
        MultipleExternalForcesQueue& inputExternalForcesQueue,
        MuscleForcesQueue& inputMuscleForcesQueue,
        EstimatedTorquesQueue& inputEstimatedTorquesQueue,
        ContactForcesQueue& outputJointReactionForcesQueue,
        Concurrency::Latch& latch,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,
        const std::vector< std::string >& jointsToLog) :
        QueuesToXForces(inputExternalForcesQueue, latch, subject, osimModelFilename, externalLoadsXmlFilename, jointsToLog),
        inputGeneralisedCoordinatesQueue_(inputGeneralisedCoordinatesQueue),
        inputMuscleForcesQueue_(inputMuscleForcesQueue),
        inputEstimatedTorquesQueue_(inputEstimatedTorquesQueue),
        outputJointReactionForcesQueue_(outputJointReactionForcesQueue)
        {}

}


#endif 