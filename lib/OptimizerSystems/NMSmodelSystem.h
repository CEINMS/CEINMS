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

#ifndef ceinms_NMSmodelSystem_h
#define ceinms_NMSmodelSystem_h

#include "AbstractOptimizerSystem.h"

#include "ParametersInterpreter.h" //TODO: rename this class
#include "TrialData.h"
#include "BatchEvaluator.h"
#include <vector>

namespace ceinms {
    template <typename NMSmodelT, typename ObjectiveFunctionT>
    class NMSmodelSystem : public Optimizers::AbstractOptimizerSystem  {
    public:
        NMSmodelSystem(NMSmodelT& subject, const std::vector<TrialData>& trialData, const ObjectiveFunctionT& objectiveFunction, const Parameter::Set& parameterSet, const std::vector<std::string>& dofsToCalibrate);

        virtual ~NMSmodelSystem() {}
        //    void setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate);
        void evaluate();
        void setParameters(const std::vector<double>& x);
        std::vector<double> getParameters();
        double getF() const { return f_; }
        size_t getNoParameters() const { return nParameters_; }
        std::vector<double> getParameters() const { return parameterInterpreter_.getSubjectParameters(); }
        void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const;
        //       void setTrialData(const std::vector<TrialData>& trialData) { trialData_ = trialData; }
        //       void pushTrialData(const TrialData& trialData) { trialData_.emplace_back(trialData); }


    private:

        size_t nParameters_;
        std::vector<NMSmodelT> mockSubjects_;
        NMSmodelT& subject_;
        BatchEvaluator batchEvaluator_;
        ObjectiveFunctionT objectiveFunction_;
        ParametersInterpreter<NMSmodelT> parameterInterpreter_;
        std::vector<std::string> dofsToCalibrate_;
        double f_;
        //      std::vector<TrialData> trialData_; //I'd like this to be here.. but it ended up in BatchEvaluator.. which has the advantage to hide
    };
}


#include "NMSmodelSystem.cpp"
#endif
