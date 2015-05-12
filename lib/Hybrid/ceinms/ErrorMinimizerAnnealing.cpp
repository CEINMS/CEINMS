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

#include "HybridWeightings.h"
#include "StaticComputation.h"
#include "StaticComputationMode/Default.h"
#include "Parameters/RecursiveEMGs.h"
#include "ObjectiveFunction/MinTorqueTrackEMGs.h"
#include "SimulatedAnnealingBase.h"

#include <iostream>
#include <stdlib.h>

namespace ceinms {
    namespace Hybrid {


        template <typename NMSmodelT>
        ErrorMinimizerAnnealing<NMSmodelT>::ErrorMinimizerAnnealing(NMSmodelT& subject)
            :subject_(subject) {

            subject_.getDoFNames(subjectDofNames_);
            currentDofNames_.resize(subjectDofNames_.size());
            currentExternalTorques_.resize(subjectDofNames_.size());
            weightings_.alpha = 1.;
            weightings_.beta = 0;
            weightings_.gamma = 0;
        }

        template<typename NMSmodelT>
        void ErrorMinimizerAnnealing<NMSmodelT>::setMusclesNamesWithEmgToTrack(const vector<string>& musclesNamesWithEmgToTrack) {

            musclesNamesWithEmgToTrack_.assign(musclesNamesWithEmgToTrack.begin(), musclesNamesWithEmgToTrack.end());
        }


        template<typename NMSmodelT>
        void ErrorMinimizerAnnealing<NMSmodelT>::setMusclesNamesWithEmgToPredict(const vector<string>& musclesNamesWithEmgToPredict) {

            musclesNamesWithEmgToPredict_.assign(musclesNamesWithEmgToPredict.begin(), musclesNamesWithEmgToPredict.end());
        }


        template<typename NMSmodelT>
        void ErrorMinimizerAnnealing<NMSmodelT>::setSingleExternalTorque(double externalTorque, const string& whichDof) {

            vector<string>::const_iterator it = subjectDofNames_.begin();
            while (*it != whichDof && it != subjectDofNames_.end())
                ++it;
            if (*it == whichDof) {
                unsigned pos = std::distance<vector<string>::const_iterator>(subjectDofNames_.begin(), it);
                currentExternalTorques_.at(pos) = externalTorque;
                currentDofNames_.at(pos) = whichDof;
            }
            else {
                std::cout << "ErrorMinimizer::setSingleExternalTorque ERROR\n" << whichDof << " not found in the subject\n";
                exit(EXIT_FAILURE);
            }

        }


        template<typename NMSmodelT>
        void ErrorMinimizerAnnealing<NMSmodelT>::setAllExternalTorques(vector<double> externalTorques, const vector<string>& dofs) {

            currentExternalTorques_ = externalTorques;
            currentDofNames_ = dofs;
        }


        template<typename NMSmodelT>
        void ErrorMinimizerAnnealing<NMSmodelT>::setAnnealingParameters(unsigned nt, unsigned ns, double rt, double t, unsigned maxNoEval, double epsilon, unsigned noEpsilon) {

            nt_ = nt;
            ns_ = ns;
            rt_ = rt;
            t_ = t;
            maxNoEval_ = maxNoEval;
            epsilon_ = epsilon;
            noEpsilon_ = noEpsilon;
        }


        template <typename NMSmodelT>
        void ErrorMinimizerAnnealing<NMSmodelT>::minimize() {
            subject_.updateState();

            if (currentDofNames_ != subjectDofNames_) {
                std::cout << "ERROR: the joints names from the external torques are different from the subject ones\n";
                exit(EXIT_FAILURE);
            }

            typedef StaticComputation<NMSmodelT, StaticComputationMode::Default<NMSmodelT> > MyStaticComputation;
            typedef SimulatedAnnealingBase <
                NMSmodelT,
                Parameters::RecursiveEMGs<NMSmodelT>,
                ObjectiveFunction::MinTorqueTrackEMGs<MyStaticComputation>,
                MyStaticComputation
            > MySiman;

            std::cout << "Optimizing timeframe " << currentTime_ << std::endl;
            MyStaticComputation staticComputation(subject_,
                musclesNamesWithEmgToTrack_,
                musclesNamesWithEmgToPredict_);
            staticComputation.setExternalTorques(currentExternalTorques_,
                currentDofNames_);
            staticComputation.setTime(currentTime_);

            MySiman siman(subject_,
                musclesNamesWithEmgToTrack_,
                musclesNamesWithEmgToPredict_,
                weightings_,
                staticComputation);
            siman.setAnnealingParameters(nt_, ns_, rt_, t_, maxNoEval_, epsilon_, noEpsilon_);
            std::cout << siman;
            siman.optimize();

            vector<double> xOpt;
            siman.getXopt(xOpt);
            cout << "final xOpt: ";
            for (unsigned i = 0; i < xOpt.size(); ++i)
                cout << xOpt.at(i) << " ";
            cout << endl;
            subject_.updateState_END_OF_HYBRID_MINIMIZATION();
        }

    };
}
