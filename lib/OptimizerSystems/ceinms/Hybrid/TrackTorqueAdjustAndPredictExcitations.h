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
        std::vector<unsigned> indexesOfMusclesWithExcitationsToAdjust_;
        std::vector<unsigned> indexesOfMusclesWithExcitationsToPredict_;
        std::vector<unsigned> indexesOfTorquesToMinimise_;
        std::vector<double> externalTorques_;
        std::vector<double> measuredExcitations_;
        double f_;
    };

    template<typename NMSmodelT>
    TrackTorqueAdjustAndPredictExcitations::TrackTorqueAdjustAndPredictExcitations(NMSmodelT& subject, const EmgInformedObjectiveFunctionSetup& setup) {

        subject.getMusclesIndexFromMusclesList(indexesOfMusclesWithExcitationsToAdjust_, setup.musclesToAdjust);
        subject.getMusclesIndexFromMusclesList(indexesOfMusclesWithExcitationsToPredict_, setup.musclesToSynthesise);
        weightings_ = setup.weightings;
        //at the moment it's all the dofs, but we may want to set that through xml
        for (unsigned i = 0; i < subject.getNoDoF(); ++i)
            indexesOfTorquesToMinimise_.push_back(i);



        //get torques to optimise from setup

    }
}

#endif