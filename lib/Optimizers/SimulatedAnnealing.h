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

#ifndef ceinms_Optimizers_SimulatedAnnealing_h
#define ceinms_Optimizers_SimulatedAnnealing_h

#include "AbstractOptimizer.h"
#include "SimulatedAnnealingParameters.h"
#include <vector>

namespace ceinms {
    namespace Optimizers {

        template<typename OptimizerSystem>
        class SimulatedAnnealing : public AbstractOptimizer  {
        public:
            SimulatedAnnealing(OptimizerSystem& optimizerSystem, SimulatedAnnealingParameters simulatedAnnealingParameters);
            ~SimulatedAnnealing() {};
            bool optimize();
            std::vector<double> getOptimizedParameters() const { return xOpt_; }

        private:
            void checkBounds(int k);
            double computeMetropolisCriteria(double t) const;
            bool doTerminate();
            void updateFandFlatest();

            OptimizerSystem& optimizerSystem_;

            std::vector<double> x_;
            std::vector<double> upperBounds_;
            std::vector<double> lowerBounds_;
            std::vector<double> xOpt_;
            std::vector<double> xp_;
            std::vector<double> v_;
            std::vector<double> fLatest_;
            std::vector<unsigned>    noAccepted_;
            int                 noParameters_;
            double              f_, fp_, fOpt_;
            //annealing parameters
            unsigned                nt_;
            unsigned                ns_;
            double                  rt_;
            double                  t_;
            unsigned                maxNoEval_;
            double                  epsilon_;
            unsigned                nEpsilon_;
        };


    }
}

#include "SimulatedAnnealing.cpp"
#endif
