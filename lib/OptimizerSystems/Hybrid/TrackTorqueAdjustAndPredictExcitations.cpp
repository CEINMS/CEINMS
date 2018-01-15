#include "ceinms/Hybrid/TrackTorqueAdjustAndPredictExcitations.h"
#include <iostream>

namespace ceinms {

    void TrackTorqueAdjustAndPredictExcitations::setExternalTorques(const std::vector<double>& externalTorques) {

        externalTorques_ = externalTorques;
    }

    void TrackTorqueAdjustAndPredictExcitations::setMeasuredExcitations(const std::vector<double>& measuredExcitations) {

        measuredExcitations_ = measuredExcitations;
    }


    void TrackTorqueAdjustAndPredictExcitations::calculate(const std::vector<double>& predictedTorques, const std::vector<double>& predictedExcitations){


        torqueError_ = 0.;
        for (auto i : indexesOfTorquesToMinimise_) {
            double diff(predictedTorques.at(i) - externalTorques_.at(i));
            torqueError_ += diff*diff/(externalTorqueErrors_.at(i)*externalTorqueErrors_.at(i));
        }
    
        excitationTrackingError_ = 0.;
        excitationSum_ = 0.;
        for (auto i : indexesOfMusclesWithExcitationsToAdjust_) {
            double diff(predictedExcitations.at(i) - measuredExcitations_.at(i));
            excitationTrackingError_ += diff*diff;// / (measuredExcitations_.at(i));
            excitationSum_ += predictedExcitations.at(i)*predictedExcitations.at(i);
        }

        for (auto i : indexesOfMusclesWithExcitationsToPredict_) {
            excitationSum_ += predictedExcitations.at(i)*predictedExcitations.at(i);
        }

        torqueError_ /= nDofs_;
        excitationSum_ /= nSynth_;
        excitationTrackingError_ /= nAdj_;
        f_ = 
            weightings_.alpha*torqueError_ + 
            weightings_.beta*excitationSum_ + 
            weightings_.gamma*excitationTrackingError_;
            
   /*     f_ =
            weightings_.alpha *torqueError_ +
            weightings_.beta*excitationSum_ +
            weightings_.gamma*excitationTrackingError_;
            */
    }

    double TrackTorqueAdjustAndPredictExcitations::getValue() const {

        return f_;
    }

    void  TrackTorqueAdjustAndPredictExcitations::getObjectiveFunctionComponents(
        double& torqueError, double& excitationSum, double& excitationTrackingError) const {

        torqueError = torqueError_;
        excitationSum = excitationSum_;
        excitationTrackingError = excitationTrackingError_;
    }

    void TrackTorqueAdjustAndPredictExcitations::
        getObjectiveFunctionComponents(std::vector<double>& components) const {

        components.resize(3);
        getObjectiveFunctionComponents(components[0], components[1], components[2]);
    }

    HybridWeightings TrackTorqueAdjustAndPredictExcitations::getWeightings() const {

        return weightings_;
    }

    void TrackTorqueAdjustAndPredictExcitations::
        getObjectiveFunctionWeightings(std::vector<double>& weightings) const {

        weightings.clear();
        weightings.push_back(weightings_.alpha);
        weightings.push_back(weightings_.beta);
        weightings.push_back(weightings_.gamma);
    }
}