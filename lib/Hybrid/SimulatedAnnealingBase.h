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
 * Author(s): D.G. Lloyd, M. Sartori, M. Reggiani                             *
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

#ifndef SimulatedAnnealingBase_h
#define SimulatedAnnealingBase_h

#include <string>
#include <vector>
#include <iostream>

#include "HybridWeightings.h"

//this is an old implementation, has to be sobstituted by the new one.. one day
namespace ceinms {
    // This is implementation is from the paper
    // Global Optimization of Statistical Functions with Simulated Annealing
    // by W. L. Goffe, G. D. Ferrier and J. Rogers
    // please refer to the papers for the meaning of the variables

    template<typename NMSmodelT, typename Parameters, typename ObjectiveFunction, typename StaticComputationT>
    class SimulatedAnnealingBase;


    template<typename NMSmodelT, typename Parameters, typename ObjectiveFunction, typename StaticComputationT>
    std::ostream& operator<< (std::ostream&, const SimulatedAnnealingBase<NMSmodelT, Parameters, ObjectiveFunction, StaticComputationT>& sa);

    template<typename NMSmodelT, typename Parameters, typename ObjectiveFunction, typename StaticComputationT>
    class SimulatedAnnealingBase {

    public:

        SimulatedAnnealingBase() {}
        SimulatedAnnealingBase(unsigned nt, unsigned ns, double rt, double t, unsigned maxNoEval);
        SimulatedAnnealingBase(NMSmodelT& mySubject,
            std::vector<std::string>& muscleNamesWithEMGtoTrack,
            std::vector<std::string>& muscleNamesWithEMGtoPredict,
            const HybridWeightings hybridParameters,
            StaticComputationT& staticComputation);

        void setAnnealingParameters(unsigned nt, unsigned ns, double rt, double t, unsigned maxNoEval, double epsilon, unsigned noEpsilon);
        void optimize();
        void getXopt(std::vector<double>& xOpt) const;

        //    friend class SimulatedAnnealingHybrid<NMSmodelT, Parameters, ObjectiveFunction, ComputationMode>;
        friend std::ostream& operator<< <> (std::ostream& output, const SimulatedAnnealingBase& sa);

    protected:
        void checkBounds(int k);

        Parameters parameters_;
        ObjectiveFunction objectiveFunction_;


        std::vector<double> x_;
        std::vector<double> upperBounds_;
        std::vector<double> lowerBounds_;
        std::vector<double> xOpt_;
        std::vector<double> xp_;
        std::vector<double> v_;
        std::vector<int>    noAccepted_;
        int                 noParameters_;

        //annealing parameters
        unsigned            nt_;
        unsigned            ns_;
        double              rt_;
        double              t_;
        unsigned            maxNoEval_;

    };

}
#include "SimulatedAnnealingBase.cpp"

#endif
