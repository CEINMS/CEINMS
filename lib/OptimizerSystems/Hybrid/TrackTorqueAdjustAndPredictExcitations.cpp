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

        double torqueError(0.);
        for (auto i : indexesOfTorquesToMinimise_) {
            double diff(predictedTorques.at(i) - externalTorques_.at(i));
            torqueError += diff*diff;
        }

        //I have to do the sum of all the excitations squared that undergo optimisation
        double excitationTrackingError(0.);
        double excitationSum(0.);

     //   std::cout << "muscle to adjust ";
        for (auto i : indexesOfMusclesWithExcitationsToAdjust_) {
       //     std::cout << i << " ";
            double diff(predictedExcitations.at(i) - measuredExcitations_.at(i));
            excitationTrackingError += diff*diff;
            excitationSum += predictedExcitations.at(i)*predictedExcitations.at(i);
        }
      //  std::cout <<  "\n";

     //   std::cout << "muscle to predict ";
        for (auto i : indexesOfMusclesWithExcitationsToPredict_) {
      //      std::cout << i << " ";
            excitationSum += predictedExcitations.at(i)*predictedExcitations.at(i);
        }

        f_ = weightings_.alpha*torqueError + weightings_.beta*excitationSum + weightings_.gamma*excitationTrackingError;
       
    /*    std::cout << "excitations: ";
        for (auto e : predictedExcitations) 
            std::cout << e << " ";
        std::cout << std::endl;
        std::cout << "sum torque error squared: " << torqueError << std::endl;
        std::cout << "sum excitations squared : " << excitationSum << std::endl;
        std::cout << "sum tracking error squared: " << excitationTrackingError << std::endl;
        
        std::cout << "f = " << f_ << std::endl;
        */
    }

    double TrackTorqueAdjustAndPredictExcitations::getValue() const {

        return f_;
    }
}