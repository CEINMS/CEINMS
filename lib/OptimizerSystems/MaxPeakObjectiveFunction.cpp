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

    void MaxPeakObjectiveFunction::calculate(const std::vector<Result>& results) {
        currentMax_.clear();
        f_ = 0;
        for (auto& r : results) {
            vector<double> maxes;
            if (setup_.getCalibrationTargetType() == CalibrationTargetType::Activation)
                maxes = calculate(r.activations);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::ScalarContactForce)
                maxes = calculate(r.scalarContactForces);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::GenericContactForce)
                maxes = calculate(r.jointReactionForces);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::NormalisedFibreLength)
                maxes = calculate(r.normalisedFibreLengths);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::NormalisedFibreVelocity)
                maxes = calculate(r.normalisedFibreVelocities);
            for(auto& m : maxes)
                f_ += setup_.getWeight() * std::pow(m, setup_.getExponent());
            currentMax_.push_back(maxes);
        }
        // normalise the weight of the objective function
        // so that the weight of the function doesn't need to change when changing the
        // number of calibrating trials
        f_ /= (results.size()*setup_.getTargetNames().size());
    }

    vector<double> MaxPeakObjectiveFunction::calculate(const DataTable<double>& result) const{
        vector<double> trialMax;
        for (auto& target : setup_.getTargetNames()) {
            //this is not very efficient, as it makes a copy of the column
            double val = 0;
            if (result.getColumnIndex(target) != -1) {
                auto col(result.getColumn(target));
                val = *std::max_element(col.begin(),col.end());
            }
            trialMax.push_back(val);
        }
        return trialMax;
    }


    vector<double> MaxPeakObjectiveFunction::calculate(const DataTable<SimTK::Vec3>& result) const {
        vector<double> trialMax;
        for (auto& target : setup_.getTargetNames()) {
            //this is not very efficient, as it makes a copy of the column
            SimTK::Vec3 val(0);
            if (result.getColumnIndex(target) != -1) {
                auto col(result.getColumn(target));
                val = *std::max_element(col.begin(), col.end(), [](auto& first, auto& second) {
                    return first.norm() < second.norm();
                });
            }
            trialMax.push_back(val.norm());
        }
        return trialMax;
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