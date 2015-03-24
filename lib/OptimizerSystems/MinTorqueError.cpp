#include "MinTorqueError.h"

#include <algorithm>
using std::begin;
using std::cbegin;
using std::end;
using std::cend;
using std::find;

#include <vector>
using std::vector;

namespace CEINMS {

    void MinTorqueError::setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate) {

        dofsToCalibrate_ = dofsToCalibrate;
        updDofsToCalibrateIdx();
    }

    void MinTorqueError::setTrials(const std::vector<TrialData>& trials) {

        trials_ = trials;
        computeVariance();
        updDofsToCalibrateIdx();
    }

    void MinTorqueError::updDofsToCalibrateIdx() {
 
        for (auto& trial : trials_) {
            dofsToCalibrateIdx_.clear();
            for (auto& dof : dofsToCalibrate_){
                auto it(find(begin(trial.dofNames), end(trial.dofNames), dof));
                if (it != end(trial.dofNames))
                    dofsToCalibrateIdx_.emplace_back(std::distance(begin(trial.dofNames), it));
            }
        }
    }

    void MinTorqueError::computeVariance() {

        auto getVariance([](const std::vector<double>& v) {
            double ans(0);
            for (auto& e : v)
                ans += e;
            auto avg(ans/v.size()); 
            auto var(0), ep(0);
            for (auto& e : v) {
                var += (e - avg)*(e - avg);
                ep += (e - avg);
            }
            return ((var - ep*ep / v.size()) / (v.size() - 1)); //adjust roundoff error.. apparently
        });


        torqueVariance_.clear();
        //trialDataVariance: first dimension is the trial, second dimension is the DoF
        for (auto& trial : trials_) {
            auto dofNamesFromTrial = trial.dofNames;
            std::vector<double> trialVariance;
            for (auto& name : dofsToCalibrate_) {
                auto i(std::distance(dofNamesFromTrial.begin(),(std::find(dofNamesFromTrial.begin(), dofNamesFromTrial.end(), name))));
                trialVariance.emplace_back(getVariance(trial.torqueData.getColumn(i)));
            }
            torqueVariance_.emplace_back(trialVariance);
        }

    }



    void MinTorqueError::calculate(const std::vector<Result>& results) {
        //reset f
        f_ = 0;
        
        using TrialDataIt = std::vector<TrialData>::const_iterator;
        using ResultIt = std::vector<Result>::const_iterator;

        class TrialCost {
        public:
            vector<double> torqueError;
            vector<double> penalty;
            TrialCost(size_t nDofs) :
                torqueError(0., nDofs),
                penalty(0., nDofs) {}
;
        };

        std::vector<TrialCost> trialCosts;
        auto tIt(cbegin(trials_));
        auto rIt(cbegin(results));
        for (tIt; tIt != cend(trials_); ++tIt, ++rIt) {
            auto trialIndex(std::distance(cbegin(trials_), tIt));
            auto diff(tIt->torqueData - rIt->torques);
            auto squaredDiff(diff*diff);

            TrialCost trialCost(tIt->noDoF);
            for (size_t r(0); r < squaredDiff.getNRows(); ++r)
                for (auto& c : dofsToCalibrateIdx_.at(trialIndex))
                    trialCost.torqueError.at(c) += squaredDiff.at(r, c);

            for (size_t r(0); r < rIt->penalties.getNRows(); ++r)
                for (auto& c : dofsToCalibrateIdx_.at(trialIndex))
                    trialCost.penalty.at(c) += rIt->penalties.at(r, c);

            return trialCosts.emplace_back(trialCost);
        };


        for (size_t trialIndex(0); trialIndex < trials_.size(); ++trialIndex) {
            for (auto& dofIdx : dofsToCalibrateIdx_.at(trialIndex))
                f_ += (trialCosts.at(trialIndex).torqueError.at(dofIdx) / torqueVariance_.at(trialIndex).at(dofIdx) +
                       trialCosts.at(trialIndex).penalty.at(dofIdx)) / trials_.at(trialIndex).torqueData.getNRows();
            }
        
    }



}