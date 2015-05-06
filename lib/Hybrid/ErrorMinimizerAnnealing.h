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

#ifndef ceinms_ErrorMinimizerAnnealing_h
#define ceinms_ErrorMinimizerAnnealing_h

#include "HybridWeightings.h"

namespace ceinms{
    namespace Hybrid {

        template<typename NMSmodelT>
        class ErrorMinimizerAnnealing {

        public:
            ErrorMinimizerAnnealing(NMSmodelT& subject);
            void setMusclesNamesWithEmgToTrack(const std::vector<std::string>& musclesNamesWithEmgToTrack);
            void setMusclesNamesWithEmgToPredict(const std::vector<std::string>& musclesNamesWithEmgToPredict);
            void setSingleExternalTorque(double externalTorque, const std::string& whichDof);
            void setAllExternalTorques(std::vector<double> externalTorques, const std::vector< std::string >& dofs);
            void setTime(double time) { currentTime_ = time; }
            void setWeightings(HybridWeightings weightings) { weightings_ = weightings; }
            void setAnnealingParameters(unsigned nt, unsigned ns, double rt, double t, unsigned maxNoEval, double epsilon, unsigned noEpsilon);
            void minimize();
        private:
            HybridWeightings weightings_;
            NMSmodelT& subject_;
            std::vector<double> currentExternalTorques_;
            std::vector<std::string> subjectDofNames_;
            std::vector<std::string> currentDofNames_;
            std::vector<std::string> musclesNamesWithEmgToTrack_;
            std::vector<std::string> musclesNamesWithEmgToPredict_;
            std::vector<double> lastActivations_;
            double currentTime_;
            unsigned            nt_;
            unsigned            ns_;
            double              rt_;
            double              t_;
            unsigned            maxNoEval_;
            double				epsilon_;
            unsigned			noEpsilon_;
        };
    }
}
#include "ErrorMinimizerAnnealing.cpp"

#endif
