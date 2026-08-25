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

#include "ceinms/MaxSumPeakObjectiveFunction.h"
#include <vector>
#include <cmath>
using std::vector;

namespace ceinms {

    MaxSumPeakObjectiveFunction::MaxSumPeakObjectiveFunction(const ObjectiveFunctionSetup& setup) :
        f_(std::numeric_limits<double>::max()),
        setup_(setup)
    {
    }

    void MaxSumPeakObjectiveFunction::setSetup(const ObjectiveFunctionSetup& setup) {
        setup_ = setup;
    }

    void MaxSumPeakObjectiveFunction::calculate(const std::vector<Result>& results) {
        currentMax_.clear();
        f_ = 0;
        for (auto& r : results) {
            double max{};
            if (setup_.getCalibrationTargetType() == CalibrationTargetType::Activation)
                max = calculate(r.activations);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::MuscleForce)
                max = calculate(r.forces);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::ScalarContactForce)
                max = calculate(r.scalarContactForces);
            //else if (setup_.getCalibrationTargetType() == CalibrationTargetType::GenericContactForce)
            //    max = calculate(r.jointReactionForces);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::NormalisedFibreLength)
                max = calculate(r.normalisedFibreLengths);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::NormalisedFibreVelocity)
                max = calculate(r.normalisedFibreVelocities);

            if (setup_.getTargetNamesRef()[0] != "none") {
                auto torques = r.torques;

                double trialMax;
                vector<double> columnsSum(torques.getNRows(), 0.);
                for (auto& target : setup_.getTargetNamesRef()) {
                    //this is not very efficient, as it makes a copy of the column
                    if (torques.getColumnIndex(target) != -1) {
                        auto col(torques.getColumn(target));
                        for (int i(0); i < columnsSum.size(); ++i) {
                            columnsSum.at(i) += col.at(i);
                        }
                    }
                }
                trialMax = *std::max_element(columnsSum.begin(), columnsSum.end());
                max /= trialMax;
            }
            f_ += setup_.getWeight() * std::pow(max, setup_.getExponent());
            currentMax_.push_back(max);
        }
        // normalise the weight of the objective function
        // so that the weight of the function doesn't need to change when changing the
        // number of calibrating trials
        f_ /= (results.size() * setup_.getTargetNames().size());        
    }

    double MaxSumPeakObjectiveFunction::calculate(const DataTable<double>& result) const {
        double trialMax;
        vector<double> columnsSum(result.getNRows(), 0.);

        for (auto& target : setup_.getTargetNames()) {
            //this is not very efficient, as it makes a copy of the column
            if (result.getColumnIndex(target) != -1) {
                auto col(result.getColumn(target));
                for (int i(0); i < columnsSum.size(); ++i) {
                    columnsSum.at(i) += col.at(i);
                }
            }        
        }

        trialMax = *std::max_element(columnsSum.begin(), columnsSum.end());

        return trialMax;
    }

    //double MaxSumPeakObjectiveFunction::calculate(const DataTable<SimTK::Vec3>& result) const {
    //    double trialMax;
    //    vector<double> columnsSum(result.getNRows(), 0.);

    //    for (auto& target : setup_.getTargetNames()) {
    //        //this is not very efficient, as it makes a copy of the column
    //        if (result.getColumnIndex(target) != -1) {
    //            auto col(result.getColumn(target));
    //            vector<double> norm(3,0.)
    //            for (int icol(0); icol < 3; ++icol){
    //              norm = *std::norm(col.at(i))
    //            }
    //            for (int i(0); i < columnsSum.size(); ++i) {
    //                double columnNorm =

    //                columnsSum.at(i) += col.at(i);
    //            }
    //        }
    //    }
    //    trialMax = *std::max_element(columnsSum.begin(), columnsSum.end());


        //for (auto& target : setup_.getTargetNames()) {
        //    //this is not very efficient, as it makes a copy of the column
        //    SimTK::Vec3 val(0);
        //    if (result.getColumnIndex(target) != -1) {
        //        auto col(result.getColumn(target));
        //        val = *std::max_element(col.begin(), col.end(), [](auto& first, auto& second) {
        //            return first.norm() < second.norm();
        //            });
        //    }
        //    trialMax.push_back(val.norm());
        //}
    //    return trialMax;
    //}

    bool MaxSumPeakObjectiveFunction::calculateOpenSimData() const {

        return setup_.getCalibrationTargetType() == CalibrationTargetType::GenericContactForce;
    }

    void MaxSumPeakObjectiveFunction::printCosts() const {
        std::cout << setup_;
        std::cout << " - Value: " << getValue() << std::endl;
        std::cout << " -- Max Peak Sum Found\n";
        for (size_t trialIndex(0); trialIndex < currentMax_.size(); ++trialIndex) {
            std::cout << " --- Trial #" << trialIndex + 1 << std::endl;
            std::cout << " ---- SumMax" << currentMax_.at(trialIndex) << std::endl;
        }
    }
}