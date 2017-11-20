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
            torqueError_ += diff*diff;
        }
        // normalise by the number of torques, so the result is independent from the model
   //     if (!indexesOfTorquesToMinimise_.empty()) {
   //         torqueError_ /= indexesOfTorquesToMinimise_.size();
    //    }
        //I have to do the sum of all the excitations squared that undergo optimisation
        excitationTrackingError_ = 0.;
        for (auto i : indexesOfMusclesWithExcitationsToAdjust_) {
            double diff(predictedExcitations.at(i) - measuredExcitations_.at(i));
            excitationTrackingError_ += diff*diff;
   //         excitationSum_ += predictedExcitations.at(i)*predictedExcitations.at(i);
        }
//        if (!indexesOfMusclesWithExcitationsToAdjust_.empty()) {
 //           excitationTrackingError_ /= indexesOfMusclesWithExcitationsToAdjust_.size();
  //      }
        excitationSum_ = 0.;
        for (auto i : indexesOfMusclesWithExcitationsToPredict_) {
            excitationSum_ += predictedExcitations.at(i)*predictedExcitations.at(i);
        }
    //    if (!indexesOfMusclesWithExcitationsToPredict_.empty()) {
     //       excitationSum_ /= indexesOfMusclesWithExcitationsToPredict_.size();
    //    }
    
        f_ = weightings_.alpha*torqueError_ + weightings_.beta*excitationSum_ + weightings_.gamma*excitationTrackingError_;
         
        //std::cout << std::endl;
        //std::cout << "sum torque error squared: " << torqueError << std::endl;
        //std::cout << "sum excitations squared : " << excitationSum << std::endl;
        //std::cout << "sum tracking error squared: " << excitationTrackingError << std::endl;
        
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