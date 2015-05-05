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
#include <string>
#include <limits>

namespace ceinms {

    template <typename NMSmodelT, typename ObjectiveFunctionT>
    NMSmodelSystem<NMSmodelT, ObjectiveFunctionT>::
        NMSmodelSystem(NMSmodelT& subject,
        const std::vector<TrialData>& trials,
        const ObjectiveFunctionT& objectiveFunction,
        const Parameter::Set& parameterSet,
        const std::vector<std::string>& dofsToCalibrate) :
        subject_(subject),
        batchEvaluator_(trials),
        objectiveFunction_(objectiveFunction),
        parameterInterpreter_(subject, parameterSet, dofsToCalibrate),
        dofsToCalibrate_(dofsToCalibrate),
        f_(std::numeric_limits<double>::max()) {

        nParameters_ = parameterInterpreter_.getNoParameters();
        //all dofs in the subject by default
        //subject_.getDoFNames(dofsToCalibrate_);
        //parameterInterpreter_.setDofsToCalibrate(dofsToCalibrate_);
        objectiveFunction_.setDofsToCalibrate(dofsToCalibrate_);
        objectiveFunction_.setTrials(trials);

        //the following is the trick for the parallel evaluation
        //and avoids copying NMSmodelT objects later on
        auto nTrials(trials.size());
        for (auto i(0.); i < nTrials; ++i)
            mockSubjects_.emplace_back(NMSmodelT(subject));


    }

    //NOTE: when using global o single parameters, only the calibrating dofs are considered
    /*   template <typename NMSmodelT, typename ObjectiveFunctionT>
       void NMSmodelSystem<NMSmodelT, ObjectiveFunctionT>::setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate) {

       dofsToCalibrate_ = dofsToCalibrate;
       parameterInterpreter_.setDofsToCalibrate(dofsToCalibrate_);
       objectiveFunction_.setDofsToCalibrate(dofsToCalibrate_);
       }
       */
    template <typename NMSmodelT, typename ObjectiveFunctionT>
    void NMSmodelSystem<NMSmodelT, ObjectiveFunctionT>::getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const {

        parameterInterpreter_.getUpperLowerBounds(upperBounds, lowerBounds);
    }

    template <typename NMSmodelT, typename ObjectiveFunctionT>
    void NMSmodelSystem<NMSmodelT, ObjectiveFunctionT>::setParameters(const std::vector<double>& x) {

        //How I think it should work..
        parameterInterpreter_.setSubjectParameters(x);
    }

    template <typename NMSmodelT, typename ObjectiveFunctionT>
    std::vector<double> NMSmodelSystem<NMSmodelT, ObjectiveFunctionT>::getParameters() {

        return parameterInterpreter_.getSubjectParameters();
    }

    template <typename NMSmodelT, typename ObjectiveFunctionT>
    void NMSmodelSystem<NMSmodelT, ObjectiveFunctionT>::evaluate() {

        //subject has the parameters, model evaluator just runs it
        batchEvaluator_.evaluateParallel(subject_, mockSubjects_);
        //batchEvaluator_.evaluateParallel(subject_);
        //       batchEvaluator_.evaluate(subject_);
        auto results(batchEvaluator_.getResults());

        objectiveFunction_.calculate(results);
        f_ = objectiveFunction_.getValue();
        //      std::cout << "f = " << f_ << endl;
        //need a way to get the breakdown of the errors from the obj function. There could be a member function that returns a vector<double>
        //and another member function that returns a vector<string> that describes the each entry of the error vector.
        //this is necessary for stats/error control..
    }
}
