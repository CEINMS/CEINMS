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

#ifndef ceinms_SumKneeContactForces_h
#define ceinms_SumKneeContactForces_h
#include <vector>
#include <string>
#include "ceinms/TrialData.h"
#include "Result.h"
#include "ceinms/ContactModelKneeBalanceSetup.h"
#include "ceinms/ContactModelKneeBalance.h"
#include "Barrier.h"
#include "ceinms/Mapper.h"
#include "ceinms/ObjectiveFunctionSetup.h"

namespace ceinms {

    class BarrierFunction {
    public:
        BarrierFunction();
        void initialize(const BarrierListSetup& setup, const std::vector<std::string>& namesFromInput);
        double getValue(const std::vector<double>& x) const;
    private:
        std::vector<Barrier> barriers_;
        std::vector<std::string> barrierTargetNames_;
        Mapper barrierMapper_;
        double f_;
    };

    class SumKneeContactForces {
    public:
        SumKneeContactForces(const ObjectiveFunctionSetup& setup);
        SumKneeContactForces() : f_(std::numeric_limits<double>::max()) {}
        void setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate) {};
        void setTrials(const std::vector<TrialData>& trials);
        void calculate(const std::vector<Result>& results);
        double getValue() const { return f_; }
        void setSetup(const ObjectiveFunctionSetup& setup);
        void setContactModelSetup(const ContactModelKneeBalanceSetup& setup);
        void setBarrierParameters(double xL, double xR, double weight, double exponent);
    private:
        double f_, weight_, exponent_;
        std::vector<TrialData> trials_;
        ContactModelKneeBalance kneeContactModel_;
        ContactModelKneeBalanceSetup kneeContactModelSetup_;
        BarrierFunction barrierFunc_;
    };

}



#endif