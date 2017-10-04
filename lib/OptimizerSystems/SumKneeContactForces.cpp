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

    SumKneeContactForces::SumKneeContactForces(const ObjectiveFunctionSetup& setup) :
        f_(std::numeric_limits<double>::max()),
        setup_(setup)
    {
    }

    void SumKneeContactForces::setSetup(const ObjectiveFunctionSetup& setup) {
        setup_ = setup;
    }

    void SumKneeContactForces::calculate(const vector<Result>& results) {
        currentMax_.clear();
        f_ = 0;
        for (auto& r : results) {
            std::vector<double> trialMax;
            for (auto& target : setup_.getTargetNames()) {
                //this is not very efficient, as it makes a copy of the column
                auto col(r.planarContactForces.getColumn(target));
                double val = *std::max_element(
                    col.begin(),
                    col.end());
                f_ += setup_.getWeight() * std::pow(val, setup_.getExponent());
                trialMax.push_back(val);
            }
            currentMax_.emplace_back(trialMax);
        }
    }

    void SumKneeContactForces::printCosts() const {

        auto names = setup_.getTargetNames();
        for (size_t trialIndex(0); trialIndex < currentMax_.size(); ++trialIndex) {
            std::cout << "Trial #" << trialIndex + 1 << std::endl;
            for (size_t i(0); i < names.size(); ++i) {
                std::cout << "max " << names.at(i) << " " << currentMax_.at(trialIndex).at(i) << " N" << std::endl;
            }
        }
        std::cout << std::endl;
        std::cout << "Fobj = " << getValue() << std::endl;
        
    }
}