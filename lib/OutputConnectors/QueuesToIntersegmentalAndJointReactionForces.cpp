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


#include "ceinms/QueuesToIntersegmentalAndJointReactionForces.h"

namespace ceinms {



    void QueuesToIntersegmentalAndJointReactionForces::operator()() {


        inputGeneralisedCoordinatesQueue_.subscribe();
        inputMuscleForcesQueue_.subscribe();
        inputEstimatedTorquesQueue_.subscribe();
        QueuesToXForces::doneWithSubscriptions();

        bool runCondition(true);
        while (runCondition) {
            auto frames(QueuesToXForces::syncFrames(inputGeneralisedCoordinatesQueue_, inputMuscleForcesQueue_, inputEstimatedTorquesQueue_));
            GeneralisedCoordinatesFrame generalisedCoordinatesFrame;
            MuscleForcesFrame muscleForcesFrame;
            EstimatedTorquesFrame estimatedTorquesFrame;
            std::tie(generalisedCoordinatesFrame, muscleForcesFrame, estimatedTorquesFrame) = frames;

            if ((generalisedCoordinatesFrame.time == std::numeric_limits<double>::infinity()) ||
                (muscleForcesFrame.time == std::numeric_limits<double>::infinity()) ||
                (estimatedTorquesFrame.time == std::numeric_limits<double>::infinity()))
                runCondition = false;
            if (runCondition) {
                QueuesToXForces::setState(
                    generalisedCoordinatesFrame.time,
                    generalisedCoordinatesFrame.data.getQ(),
                    generalisedCoordinatesFrame.data.getQd(),
                    generalisedCoordinatesFrame.data.getQdd()
                    );

                QueuesToXForces::calculateJointReactionForces(muscleForcesFrame.data, estimatedTorquesFrame.data);
                ContactForcesFrame jrForces;
                jrForces.time = generalisedCoordinatesFrame.time;
                QueuesToXForces::getJointReactionForcesInG(jrForces.data);
                outputJointReactionForcesQueue_.push(jrForces);

                QueuesToXForces::calculateIntersegmentalForces();
                ContactForcesFrame intForces;
                intForces.time = generalisedCoordinatesFrame.time;
                QueuesToXForces::getIntersegmentalForcesInG(intForces.data);
                outputIntersegmentalForcesQueue_.push(intForces);

            }
            ContactForcesFrame endOfData;
            endOfData.time = std::numeric_limits<double>::infinity();
            outputJointReactionForcesQueue_.push(endOfData);
            outputIntersegmentalForcesQueue_.push(endOfData);
        }
    }
}