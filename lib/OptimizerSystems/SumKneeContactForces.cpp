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


    BarrierFunction::BarrierFunction() : f_(std::numeric_limits<double>::max()) {}

    void BarrierFunction::initialize(const BarrierListSetup& setup, const std::vector<std::string>& namesFromInput) {


        for (auto it : setup.barrierSetup) {
            barriers_.push_back(Barrier(it.xL, it.xR, it.weight, it.exponent));
            barrierTargetNames_.push_back(it.targetName);
        }
        barrierMapper_.setNames(namesFromInput, barrierTargetNames_);
    }


    double BarrierFunction::getValue(const std::vector<double>& x) const {

        double f = 0;
        auto orderedX = barrierMapper_.map(x);
        for (unsigned i(0); i < barriers_.size(); ++i)
            f += barriers_.at(i).getPenalty(orderedX.at(i));
        return f;
    }


    SumKneeContactForces::SumKneeContactForces(const ObjectiveFunctionSetup& setup) : f_(std::numeric_limits<double>::max()) 
    {
        setSetup(setup);
        barrierFunc_.initialize(setup.barrierListSetup, kneeContactModel_.getCondyleNames());
    
    }

    void SumKneeContactForces::setTrials(const vector<TrialData>& trials) {

        trials_ = trials;
        kneeContactModel_.initialize(kneeContactModelSetup_, trials_.front().dofNames);
    }

    void SumKneeContactForces::setContactModelSetup(const ContactModelKneeBalanceSetup& setup) {

        kneeContactModelSetup_ = setup;
        if (!trials_.empty())
            kneeContactModel_.initialize(kneeContactModelSetup_, trials_.front().dofNames);

    }


    void SumKneeContactForces::setSetup(const ObjectiveFunctionSetup& setup) {

        
        weight_ = setup.weight;
        exponent_ = setup.exponent;

    }


    void SumKneeContactForces::calculate(const vector<Result>& results) {

        f_ = 0;
        vector<double> contactForces;
        auto tIt(trials_.cbegin());
        auto rIt(results.cbegin());
        for (tIt; tIt != trials_.cend(); ++tIt, ++rIt) {
            for (size_t rowIdx(0); rowIdx < tIt->torqueData.getNRows(); ++rowIdx) {
                kneeContactModel_.setTorques(tIt->torqueData.getRow(rowIdx), rIt->torques.getRow(rowIdx));
                kneeContactModel_.getContactForces(contactForces);
                for (auto& cf : contactForces)
                    f_ += weight_*std::pow(cf, exponent_);
                f_ += barrierFunc_.getValue(contactForces);
            }

        }

    }

}