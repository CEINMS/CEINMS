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

#include "ceinms/MaxPeakObjectiveFunction.h"
#include <vector>
#include <cmath>
using std::vector;

namespace ceinms {

    MaxPeakObjectiveFunction::MaxPeakObjectiveFunction(const ObjectiveFunctionSetup& setup) :
        f_(std::numeric_limits<double>::max()),
        setup_(setup)
    {
    }

    void MaxPeakObjectiveFunction::setSetup(const ObjectiveFunctionSetup& setup) {
        setup_ = setup;
    }

    void MaxPeakObjectiveFunction::calculate(const vector<Result>& results) {
        currentMax_.clear();
        f_ = 0;
        for (auto& r : results) {
            std::vector<double> trialMax;
            for (auto& target : setup_.getTargetNames()) {
                //this is not very efficient, as it makes a copy of the column
                auto col(getDataTable(r).getColumn(target));
                double val = *std::max_element(
                    col.begin(),
                    col.end());
                f_ += setup_.getWeight() * std::pow(val, setup_.getExponent());
                trialMax.push_back(val);
            }
            currentMax_.emplace_back(trialMax);
        }
        // normalise the weight of the objective function
        // so that the weight of the function doesn't need to change when changing the
        // number of calibrating trials
        f_ /= (results.size()*setup_.getTargetNames().size());
    }

    const DataTable<double>& MaxPeakObjectiveFunction::getDataTable(const Result& result) const {
       
        switch (setup_.getCalibrationTargetType()) {
        case CalibrationTargetType::Activation:
            return result.activations;
     //   case CalibrationTargetType::FibreLength:
     //       return result
     //   case CalibrationTargetType::FibreVelocity:
     //       break;
     //   case CalibrationTargetType::PennationAngle:
     //       break;
        case CalibrationTargetType::PlanarContactForces:
            return result.planarContactForces;
        case CalibrationTargetType::Torque:
            return result.torques;
        default:
            throw std::exception("MaxPeakObjectiveFunction::getDataTable(const Result& result)\n"
                "data not available");
        }
        //this should never happen
        return result.penalties;

    }

    void MaxPeakObjectiveFunction::printCosts() const {
        std::cout << setup_;
        std::cout << " - Value: " << getValue() << std::endl;
        std::cout << " -- Max Peaks Found\n";
        for (size_t trialIndex(0); trialIndex < currentMax_.size(); ++trialIndex) {
            std::cout << " --- Trial #" << trialIndex + 1 << std::endl;
            for (size_t i(0); i < setup_.getTargetNames().size(); ++i) {
                std::cout << " ---- " << setup_.getTargetNames().at(i) << ": " << currentMax_.at(trialIndex).at(i) << std::endl;
            }
        }
    }
}