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

#include "ceinms/SumKneeContactForces.h"
#include <vector>
#include <cmath>
using std::vector;

namespace ceinms {


    void SumKneeContactForces::setTrials(const vector<TrialData>& trials) {

        trials_ = trials;
        kneeContactModel_.initialize(kneeContactModelSetup_, trials_.front().dofNames);
    }

    void SumKneeContactForces::setContactModelSetup(const ContactModelKneeBalanceSetup& setup) {

        kneeContactModelSetup_ = setup;
        if (!trials_.empty())
            kneeContactModel_.initialize(kneeContactModelSetup_, trials_.front().dofNames);
    }


    void SumKneeContactForces::setBarrierParameters(double xL, double xR, double weight, double exponent) {

        barrier_.setParameters(xL, xR, weight, exponent);
    }


    void SumKneeContactForces::calculate(const vector<Result>& results) {

        f_ = 0;
        double weight1, weight2, exponent1, exponent2;
        vector<double> contactForces;
        auto tIt(trials_.cbegin());
        auto rIt(results.cbegin());
        for (tIt; tIt != trials_.cend(); ++tIt, ++rIt) {
            for (size_t rowIdx(0); rowIdx < tIt->torqueData.getNRows(); rowIdx) {
                kneeContactModel_.setTorques(tIt->torqueData.getRow(rowIdx), rIt->torques.getRow(rowIdx));
                kneeContactModel_.getContactForces(contactForces);
                f_ += weight1*std::pow(contactForces.at(0), exponent1) +
                    weight2*std::pow(contactForces.at(1), exponent2) +
                    barrier_.getPenalty(contactForces.at(0)) + barrier_.getPenalty(contactForces.at(1));

            }

        }

    }

}