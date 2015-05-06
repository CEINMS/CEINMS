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

#ifndef ceinms_BatchEvaluator_h
#define ceinms_BatchEvaluator_h

#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "MuscleParameters.h"
#include "DataTable.h"
#include "TrialData.h"
#include "Result.h"

namespace ceinms {

    class OpenLoopEvaluator {
    public:
        OpenLoopEvaluator() = delete;
        template<typename NMSmodelT>
        static void evaluate(NMSmodelT&& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult);
    private:
        template<typename NMSmodelT>
        static void initFiberLengthTraceCurves(NMSmodelT&& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult);
    };


    class BatchEvaluator{

    public:
        using EvaluationResult = Result; //just to expose it
        BatchEvaluator(const std::vector<TrialData>& trials);
        template<typename NMSmodelT>
        void evaluate(NMSmodelT& subject);
        template<typename NMSmodelT>
        void evaluateParallel(NMSmodelT& subject);
        template<typename NMSmodelT>
        void evaluateParallel(NMSmodelT& subject, std::vector<NMSmodelT>& mockSubjects);

        std::vector<Result> getResults() const {
            return results_;
        }

    private:
        void updMusclesToUpdate();
        std::vector<MuscleParameters> subjectParameters_, subjectParametersT1_; //this should be something like typename NMSmodelT::MTUtype::MuscleParameters.. since it should depend on the muscle model we are using
        std::vector<unsigned> musclesToUpdate_;
        std::vector<TrialData> trials_;
        std::vector<Result> results_;
    };
}

#include "BatchEvaluator.cpp"
#endif
