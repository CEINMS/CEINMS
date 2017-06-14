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

#include "ceinms/SumGenericContactForces.h"
#include <vector>
#include <cmath>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <algorithm>
#include <numeric>

namespace ceinms {


    SumGenericContactForces::SumGenericContactForces() : 
        f_(std::numeric_limits<double>::max()),
        weight_(1.),
        exponent_(1.)
    {    }


    SumGenericContactForces::SumGenericContactForces(const ObjectiveFunctionSetup& setup) :
        SumGenericContactForces()
    {
        setSetup(setup);

    }

    void SumGenericContactForces::setSetup(const ObjectiveFunctionSetup& setup) {

        weight_ = setup.weight;
        exponent_ = setup.exponent;
        barrierListSetup_ = setup.barrierListSetup;

    }

    void SumGenericContactForces::setJointContactForcesNames(const std::vector<std::string>& contactForcesNames) {

        barrierFunc_.initialize(barrierListSetup_, contactForcesNames);
        contactForcesNames_ = contactForcesNames;
    }


    void SumGenericContactForces::calculate(const vector<Result>& results) {
        maxContactForces_.clear();
        for (auto& r : results) {

            std::vector<double> maxContactForcesForCurrentResult;
            for (auto& joint : contactForcesNames_) {
                auto column(r.jointReactionForces.getColumn(joint));
                std::vector<double> magnitude;
                std::transform(column.cbegin(), column.cend(), std::back_inserter(magnitude), [](const SimTK::Vec3& v) {
                    return v.norm();
                });
                auto it(std::max_element(magnitude.cbegin(), magnitude.cend()));
                maxContactForcesForCurrentResult.emplace_back(*it);
            }
            maxContactForces_.emplace_back(maxContactForcesForCurrentResult);
        }

        f_ = 0.;
        for (auto& v : maxContactForces_) {
            f_ += barrierFunc_.getValue(v);
            for (auto& e : v) {
                f_ += e;
            }
        }
        f_ = weight_*std::pow(f_, exponent_);
    }

    void SumGenericContactForces::printCosts() const {

        cout << "Peak contact forces:" << endl;
        for (int i(0); i < maxContactForces_.size(); ++i) {
            cout << "Trial# " << i+1 << endl;
            for (int j(0); j < contactForcesNames_.size(); ++j) {
                cout << contactForcesNames_.at(j) << " " << maxContactForces_.at(i).at(j) << " N" << endl;
            }
        }
        std::cout << "Fobj = " << getValue() << std::endl;
    }
}
  
