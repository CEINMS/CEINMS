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
 * Author(s): C. Pizzolato, M. Reggiani, D.G. Lloyd                           *
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

#include "ceinms/TorqueErrorNormalisedObjectiveFunction.h"
#include <algorithm>
using std::begin;
using std::end;
using std::find;
#include <vector>
using std::vector;

namespace ceinms {

    TorqueErrorNormalisedObjectiveFunction::TorqueErrorNormalisedObjectiveFunction(const ObjectiveFunctionSetup& setup) :
        f_(std::numeric_limits<double>::max()),
        setup_(setup) {
        dofsToCalibrate_ = setup_.getTargetNames();
        updDofsToCalibrateIdx();
    }

    void TorqueErrorNormalisedObjectiveFunction::setSetup(const ObjectiveFunctionSetup & setup) {
        setup_ = setup;
        dofsToCalibrate_ = setup_.getTargetNames();
        f_ = std::numeric_limits<double>::max();
        computeVariance();
        updDofsToCalibrateIdx();
    }

    void TorqueErrorNormalisedObjectiveFunction::setTrials(const std::vector<TrialData>& trials) {

        trials_ = trials;
        computeVariance();
        updDofsToCalibrateIdx();
    }

    void TorqueErrorNormalisedObjectiveFunction::updDofsToCalibrateIdx() {
        dofsToCalibrateIdx_.clear();
        for (auto& trial : trials_) {
            auto dofNames(trial.getDofNames());
            vector<unsigned> indeces;
            for (auto& dof : dofsToCalibrate_){
                auto it(find(begin(dofNames), end(dofNames), dof));
                if (it != end(dofNames))
                    indeces.emplace_back(std::distance(begin(dofNames), it));
            }
            dofsToCalibrateIdx_.emplace_back(indeces);
        }
    }

    void TorqueErrorNormalisedObjectiveFunction::computeVariance() {

        auto getVariance([](const std::vector<double>& v) {
            double ans(0);
            for (auto& e : v)
                ans += e;
            double avg(ans / v.size());
            double var(0), ep(0);
            for (auto& e : v) {
                var += (e - avg)*(e - avg);
                ep += (e - avg);
            }
            return ((var - ep*ep / v.size()) / (v.size() - 1)); //adjust roundoff error
        });

        torqueVariance_.clear();
        //trialDataVariance: first dimension is the trial, second dimension is the DoF
        for (auto& trial : trials_) {
            auto dofNamesFromTrial = trial.getDofNames();
            std::vector<double> trialVariance(trial.getNoDofs(), .0);
            for (auto& name : dofsToCalibrate_) {
                auto i(std::distance(dofNamesFromTrial.begin(), (std::find(dofNamesFromTrial.begin(), dofNamesFromTrial.end(), name))));
                trialVariance.at(i) = getVariance(trial.getTorqueData().getColumn(i));
                if (trialVariance.at(i) < 1) trialVariance.at(i) = 1;   //don't penalize if the variance in low
            }
            torqueVariance_.emplace_back(trialVariance);
        }
    }

    void TorqueErrorNormalisedObjectiveFunction::calculate(const std::vector<Result>& results) {
        //reset f
        f_ = 0;

        using TrialDataIt = std::vector<TrialData>::const_iterator;
        using ResultIt = std::vector<Result>::const_iterator;

        trialCosts_.clear();
        auto tIt(trials_.cbegin());
        auto rIt(results.cbegin());
        for (tIt; tIt != trials_.cend(); ++tIt, ++rIt) {
            auto trialIndex(std::distance(trials_.cbegin(), tIt));
            auto diff(tIt->getTorqueData() - rIt->torques);
            auto squaredDiff(diff*diff);
            TrialCost trialCost(tIt->getNoDofs());
            for (size_t r(0); r < squaredDiff.getNRows(); ++r) {
                for (auto& c : dofsToCalibrateIdx_.at(trialIndex)) {
                    trialCost.torqueError.at(c) += squaredDiff.at(r, c);
                }
            }
            for (size_t r(0); r < rIt->penalties.getNRows(); ++r)
                for (size_t c(0); c < rIt->penalties.getNColumns(); ++c)
                    trialCost.penalty += rIt->penalties.at(r, c);
            trialCosts_.emplace_back(trialCost);
        }
        for (size_t trialIndex(0); trialIndex < trials_.size(); ++trialIndex) {
            for (auto& dofIdx : dofsToCalibrateIdx_.at(trialIndex))
                f_ += (trialCosts_.at(trialIndex).torqueError.at(dofIdx) / torqueVariance_.at(trialIndex).at(dofIdx) +
                trialCosts_.at(trialIndex).penalty) / results.at(trialIndex).torques.getNRows();
        }
        f_ *= setup_.getWeight();
    }

    void TorqueErrorNormalisedObjectiveFunction::printCosts() const {

        std::cout << setup_;
        std::cout << " - Value: " << getValue() << std::endl;
        std::cout << " -- Root Mean Square Errors\n";
        for (size_t trialIndex(0); trialIndex < trials_.size(); ++trialIndex) {
            std::cout <<" --- Trial #" << trialIndex+1 << std::endl;
            for (auto& dofIdx : dofsToCalibrateIdx_.at(trialIndex)) {
                        double val = sqrt(trialCosts_.at(trialIndex).torqueError.at(dofIdx) / 
                            trials_.at(trialIndex).getTorqueData().getNRows());
                        std::cout << " ---- " << trials_.at(trialIndex).getDofNames().at(dofIdx)  << ": " << val << " Nm" << std::endl;
            }
        }
    }
}