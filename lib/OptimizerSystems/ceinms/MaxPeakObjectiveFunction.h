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

#ifndef ceinms_MaxPeakObjectiveFunction_h
#define ceinms_MaxPeakObjectiveFunction_h
#include <vector>
#include <string>
#include "ceinms/TrialData.h"
#include "Result.h"
#include "ceinms/ObjectiveFunctionSetup.h"
#include "ceinms/ObjectiveFunctionBase.h"

namespace ceinms {

    // For each target name and each results, finds the max peak. 
    // Peaks are elevated to the exponent specified in the setup
    // and the summed. This value is then averaged by the number of 
    // total peaks found and multiplied by the weight
    class MaxPeakObjectiveFunction : public ObjectiveFunctionBase {
    public:
        MaxPeakObjectiveFunction(const ObjectiveFunctionSetup& setup);
        MaxPeakObjectiveFunction() : f_(std::numeric_limits<double>::max()) {}
        void setSetup(const ObjectiveFunctionSetup& setup);
        void setTrials(const std::vector<TrialData>& trials) override { /* blank */}
        void calculate(const std::vector<Result>& results) override;
        double getValue() const override { return f_; }
        void printCosts() const override;
		bool calculateOpenSimData() const override;
    private:
        std::vector<double> calculate(const DataTable<double>& result) const;
        std::vector<double> calculate(const DataTable<SimTK::Vec3>& result) const;
        ObjectiveFunctionSetup setup_;
        double f_;
        std::vector<std::vector<double>> currentMax_; //trial, max contact force value
    };
}

#endif