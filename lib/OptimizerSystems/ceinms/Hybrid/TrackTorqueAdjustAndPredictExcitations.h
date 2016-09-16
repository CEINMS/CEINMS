#ifndef ceinms_TrackTorqueAdjustAndPredictExcitations_h
#define ceinms_TrackTorqueAdjustAndPredictExcitations_h

#include <vector>

#include "ceinms/EmgInformedObjectiveFunctionSetup.h"

namespace ceinms {

    class TrackTorqueAdjustAndPredictExcitations{

    public:
        template<typename NMSmodelT>
        TrackTorqueAdjustAndPredictExcitations(NMSmodelT& subject, const EmgInformedObjectiveFunctionSetup& setup);
        void setExternalTorques(const std::vector<double>& externalTorques);
        void setMeasuredExcitations(const std::vector<double>& measuredExcitations);
        void calculate(const std::vector<double>& predictedTorques, const std::vector<double>& predictedExcitations);
        double getValue() const;
    private:
        HybridWeightings weightings_;
        std::vector<unsigned> indexesOfMusclesWithExcitationsToAdjust;
        std::vector<unsigned> indexesOfMusclesWithExcitationsToPredict;
        std::vector<unsigned> indexesOfTorquesToMinimise;
        std::vector<double> externalTorques_;
        std::vector<double> measuredExcitations_;
        double f_;
    };

    template<typename NMSmodelT>
    TrackTorqueAdjustAndPredictExcitations::TrackTorqueAdjustAndPredictExcitations(NMSmodelT& subject, const EmgInformedObjectiveFunctionSetup& setup) {

        subject.getMusclesIndexFromMusclesList(indexesOfMusclesWithExcitationsToAdjust, setup.musclesToAdjust);
        subject.getMusclesIndexFromMusclesList(indexesOfMusclesWithExcitationsToPredict, setup.musclesToSynthesise);
        weightings_ = setup.weightings;
        //get torques to optimise from setup

    }
}

#endif