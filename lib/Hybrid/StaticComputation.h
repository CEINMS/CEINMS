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
 
#ifndef ceinms_Hybrid_StaticComputation_h
#define ceinms_Hybrid_StaticComputation_h

#include <vector>
#include "NMSmodel.h"

namespace ceinms {
    namespace Hybrid {

        template<typename NMSmodelT, typename StaticComputationMode>
        class StaticComputation;

        template<typename NMSmodelT, typename StaticComputationMode>
        std::ostream& operator<< (std::ostream& output, const StaticComputation<NMSmodelT, StaticComputationMode>& m);

        template<typename NMSmodelT, typename StaticComputationMode>
        class StaticComputation {

        public:
            StaticComputation(NMSmodelT& subject,
                const std::vector<std::string>& muscleNamesWithEMGtoTrack,
                const std::vector<std::string>& muscleNamesWithEMGtoPredict);
            void setTime(double time) { time_ = time; computationMode_.setTime(time); }
            void setExternalTorques(const std::vector<double>& externalTorques, const std::vector<std::string>& dofNames);
            void getExternalTorques(std::vector<double>& externalTorques) const;
            void getTorques(std::vector<double>& torques);
            void getInitialValuesOfTrackedEMGs(std::vector<double>& emgValues) const;
            void getAdjustedValuesOfTrackedEMGs(std::vector<double>& emgValues) const;
            void getCurrentEMGs(std::vector<double>& emgValues) const;

        private:

            NMSmodelT& subject_;
            StaticComputationMode computationMode_;
            double time_;
            std::vector<double> externalTorques_;
            std::vector<double> initialTorques_;   //torques at the start of the frame, before optimization
            std::vector<double> initialValueOfTrackedEMGs_;
            std::vector<unsigned> muscleIndexWithEMGtoTrack_;
            std::vector<unsigned> muscleIndexWithEMGtoPredict_;
            std::vector<unsigned> muscleIndexWithEMGtoOptimize_;


        };
    }
}
#include "StaticComputation.cpp"

#endif
