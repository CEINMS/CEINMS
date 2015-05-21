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

#include "ceinms/MinTorqueErrorAndContactForces.h"
#include <vector>
using std::vector;
#include <string>
using std::string;

namespace ceinms {

    MinTorqueErrorAndContactForces::MinTorqueErrorAndContactForces(const MinTorqueError& torqueErrorFun, const SumKneeContactForces& sumKneeContactFun) :
        minTorqueError_(torqueErrorFun),
        sumKneeContactForces_(sumKneeContactFun),
        f_(std::numeric_limits<double>::max()) {}
   
    
    //MinTorqueErrorAndContactForces::MinTorqueErrorAndContactForces() : f_(std::numeric_limits<double>::max()) {}

    void MinTorqueErrorAndContactForces::setDofsToCalibrate(const vector<string>& dofsToCalibrate) {

        dofsToCalibrate_ = dofsToCalibrate;
        minTorqueError_.setDofsToCalibrate(dofsToCalibrate_);
        sumKneeContactForces_.setDofsToCalibrate(dofsToCalibrate_);
    }

    void MinTorqueErrorAndContactForces::setTrials(const vector<TrialData>& trials) {

        trials_ = trials;
        minTorqueError_.setTrials(trials_);
        sumKneeContactForces_.setTrials(trials_);
    }

    void MinTorqueErrorAndContactForces::calculate(const vector<Result>& results) {

        f_ = 0;
        minTorqueError_.calculate(results);
        f_ += minTorqueError_.getValue();
        sumKneeContactForces_.calculate(results);
        f_ += sumKneeContactForces_.getValue();
    }

}