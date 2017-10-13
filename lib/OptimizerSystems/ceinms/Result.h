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

#ifndef ceinms_Result_h
#define ceinms_Result_h

#include <OpenSim/OpenSim.h>
#include "ceinms/DataTable.h"

namespace ceinms {
    class Result{
    public:
        DataTable<double> forces, torques, activations, emgs, penalties, errors, planarContactForces,
            normalisedFibreLengths, normalisedFibreVelocities;
        DataTable<SimTK::Vec3> jointReactionForces, jointReactionTorques;
        Result(size_t nMTUs, size_t nDofs, size_t nEmgFrames, size_t nLmtMaFrames) :
            forces(nLmtMaFrames, nMTUs),
            torques(nLmtMaFrames, nDofs),
            activations(nEmgFrames, nMTUs),
            emgs(nEmgFrames, nMTUs),
            penalties(nLmtMaFrames, nMTUs),
            planarContactForces(nLmtMaFrames, 4),
            normalisedFibreLengths(nLmtMaFrames, nMTUs),
            normalisedFibreVelocities(nLmtMaFrames, nMTUs),
            errors(nLmtMaFrames, 6){

            errors.setLabels({ "TorqueError", "SumExcitation", "ExcitationTrackingError", "alpha", "beta", "gamma" });
        }

        void setMuscleNames(std::vector<std::string> muscleNames) {
            forces.setLabels(muscleNames);
            activations.setLabels(muscleNames);
            emgs.setLabels(muscleNames);
            penalties.setLabels(muscleNames);
        }

        void setDofNames(std::vector<std::string> dofNames) {
            torques.setLabels(dofNames);
        }

        // any change in force would actually reflect changes in other quantities
        bool equals(const Result& rhs) const {
            return (forces == rhs.forces      &&
                torques == rhs.torques     &&
                activations == rhs.activations &&
                emgs == rhs.emgs &&
                penalties == rhs.penalties);
        }
    };
}
#endif
