#include "ceinms/Hybrid/TrackTorqueAdjustAndPredictExcitations.h"

namespace ceinms {

    void TrackTorqueAdjustAndPredictExcitations::setExternalTorques(const std::vector<double>& externalTorques) {

        externalTorques_ = externalTorques;
    }

    void TrackTorqueAdjustAndPredictExcitations::setMeasuredExcitations(const std::vector<double>& measuredExcitations) {

        measuredExcitations_ = measuredExcitations;
    }


    void TrackTorqueAdjustAndPredictExcitations::calculate(const std::vector<double>& predictedTorques, const std::vector<double>& predictedExcitations){

        double torqueError(0.);
        for (auto i : indexesOfTorquesToMinimise) {
            double diff(predictedTorques.at(i) - externalTorques_.at(i));
            torqueError += diff*diff;
        }

        //I have to do the sum of all the excitations squared that undergo optimisation
        double excitationTrackingError(0.);
        double excitationSum(0.);

        for (auto i : indexesOfMusclesWithExcitationsToAdjust) {
            double diff(predictedExcitations.at(i) - measuredExcitations_.at(i));
            excitationTrackingError += diff*diff;
            excitationSum += measuredExcitations_.at(i)*measuredExcitations_.at(i);
        }

        for (auto i : indexesOfMusclesWithExcitationsToPredict) {
            excitationSum += measuredExcitations_.at(i)*measuredExcitations_.at(i);
        }

        f_ = weightings_.alpha*torqueError + weightings_.beta*excitationSum + weightings_.gamma*excitationTrackingError;

    }

    double TrackTorqueAdjustAndPredictExcitations::getValue() const {

        return f_;
    }
}