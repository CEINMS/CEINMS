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
 * Author(s):                                                                 *
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

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;

#include <stdlib.h>

namespace ceinms {
    namespace Hybrid {

        //TODO:: ricordarsi di far iniziare i muscoli da predirre con il valore al tempo t1
        template<typename NMSmodelT, typename StaticComputationMode>
        StaticComputation<NMSmodelT, StaticComputationMode>::StaticComputation(NMSmodelT& subject,
            const vector<string>& muscleNamesWithEMGtoTrack,
            const vector<string>& muscleNamesWithEMGtoPredict) :
            subject_(subject), computationMode_(subject) {

            if (!subject_.haveTheseMuscles(muscleNamesWithEMGtoTrack) || !subject_.haveTheseMuscles(muscleNamesWithEMGtoPredict)) {
                std::cout << "Error in RecursiveEMGs_Hybrid: muscle name not found\n";
                exit(EXIT_FAILURE);
            }

            subject_.getMusclesIndexFromMusclesList(muscleIndexWithEMGtoTrack_, muscleNamesWithEMGtoTrack);
            subject_.getMusclesIndexFromMusclesList(muscleIndexWithEMGtoPredict_, muscleNamesWithEMGtoPredict);

            //concatenate muscleIndexWithEMGtoTrack_ and muscleIndexWithEMGtoPredict_
            muscleIndexWithEMGtoOptimize_.assign(muscleIndexWithEMGtoTrack_.begin(), muscleIndexWithEMGtoTrack_.end());
            muscleIndexWithEMGtoOptimize_.insert(muscleIndexWithEMGtoOptimize_.end(), muscleIndexWithEMGtoPredict_.begin(), muscleIndexWithEMGtoPredict_.end());

            vector<double> currentEMGData;
            subject_.getEmgs(currentEMGData);

            for (unsigned i = 0; i < muscleIndexWithEMGtoTrack_.size(); ++i)
                initialValueOfTrackedEMGs_.push_back(currentEMGData.at(muscleIndexWithEMGtoTrack_.at(i)));
        }


        template<typename NMSmodelT, typename StaticComputationMode>
        void StaticComputation<NMSmodelT, StaticComputationMode>::setExternalTorques(const vector<double>& externalTorques, const vector<string>& dofNames) {

            vector<string> dofNamesFromSubject;
            subject_.getDoFNames(dofNamesFromSubject);
            if (dofNamesFromSubject != dofNames)
            {
                cout << "StaticComputation: dofNames from the subject and external torques dof names are different\n";
                exit(EXIT_FAILURE);
            }

            externalTorques_ = externalTorques;
        }


        template<typename NMSmodelT, typename StaticComputationMode>
        void StaticComputation<NMSmodelT, StaticComputationMode>::getExternalTorques(vector<double>& externalTorques) const {

            externalTorques = externalTorques_;
        }


        template<typename NMSmodelT, typename StaticComputationMode>
        void StaticComputation<NMSmodelT, StaticComputationMode>::getTorques(vector<double>& torques) {

            computationMode_.getTorques(torques);
        }


        template<typename NMSmodelT, typename StaticComputationMode>
        void StaticComputation<NMSmodelT, StaticComputationMode>::getCurrentEMGs(vector<double>& emgValues) const {

            subject_.getEmgs(emgValues);
        }


        template<typename NMSmodelT, typename StaticComputationMode>
        void StaticComputation<NMSmodelT, StaticComputationMode>::getInitialValuesOfTrackedEMGs(vector<double>& emgValues) const {

            emgValues.assign(initialValueOfTrackedEMGs_.begin(), initialValueOfTrackedEMGs_.end());
        }


        template<typename NMSmodelT, typename StaticComputationMode>
        void StaticComputation<NMSmodelT, StaticComputationMode>::getAdjustedValuesOfTrackedEMGs(vector<double>& emgValues) const {

            emgValues.resize(muscleIndexWithEMGtoTrack_.size());
            vector<double> currentEMGData;
            subject_.getEmgs(currentEMGData);

            for (unsigned i = 0; i < muscleIndexWithEMGtoTrack_.size(); ++i)
                emgValues.at(i) = currentEMGData.at(muscleIndexWithEMGtoTrack_.at(i));

        }

    }
}
