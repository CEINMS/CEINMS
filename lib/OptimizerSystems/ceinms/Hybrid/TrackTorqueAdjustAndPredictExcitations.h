#ifndef ceinms_TrackTorqueAdjustAndPredictExcitations_h
#define ceinms_TrackTorqueAdjustAndPredictExcitations_h

#include <vector>
#include <algorithm>

#include "ceinms/EmgInformedObjectiveFunctionSetup.h"
#include "ceinms/Mapper.h"

namespace ceinms {

    class TrackTorqueAdjustAndPredictExcitations{

    public:
        template<typename NMSmodelT>
        TrackTorqueAdjustAndPredictExcitations(NMSmodelT& subject, const EmgInformedObjectiveFunctionSetup& setup);
        void setExternalTorques(const std::vector<double>& externalTorques);
        void setMeasuredExcitations(const std::vector<double>& measuredExcitations);
        void calculate(const std::vector<double>& predictedTorques, const std::vector<double>& predictedExcitations);
        double getValue() const;
        void getObjectiveFunctionComponents(double& torqueError, double& excitationSum, double& excitationTrackingError) const;
        void getObjectiveFunctionComponents(std::vector<double>& components) const;
        HybridWeightings getWeightings() const;
        void getObjectiveFunctionWeightings(std::vector<double>& weightings) const;
    private:
        HybridWeightings weightings_;
        std::vector<unsigned> indexesOfMusclesWithExcitationsToAdjust_;
        std::vector<unsigned> indexesOfMusclesWithExcitationsToPredict_;
        std::vector<unsigned> indexesOfTorquesToMinimise_;
        std::vector<double> externalTorques_;
        std::vector<double> measuredExcitations_;
        double f_;
        double torqueError_;
        double excitationSum_;
        double excitationTrackingError_;

    };

    template<typename NMSmodelT>
    TrackTorqueAdjustAndPredictExcitations::TrackTorqueAdjustAndPredictExcitations(NMSmodelT& subject, const EmgInformedObjectiveFunctionSetup& setup) {

        std::vector<std::string> musclesToAdjust(setup.musclesToAdjust);
        std::vector<std::string> musclesToSynthesise(setup.musclesToSynthesise);

        //remove possible duplicate entries
        std::sort(std::begin(musclesToAdjust), std::end(musclesToAdjust));
        std::sort(std::begin(musclesToSynthesise), std::end(musclesToSynthesise));
        std::unique(std::begin(musclesToAdjust), std::end(musclesToAdjust));
        std::unique(std::begin(musclesToSynthesise), std::end(musclesToSynthesise));
        std::vector<std::string> duplicates;
        std::set_intersection(
            std::begin(musclesToAdjust),
            std::end(musclesToAdjust),
            std::begin(musclesToSynthesise),
            std::end(musclesToSynthesise),
            std::back_inserter(duplicates));
        //remove duplicates from synthMTUs
        std::vector<std::string> uniqueIndexes;;
        std::set_difference(
            std::begin(musclesToSynthesise),
            std::end(musclesToSynthesise),
            std::begin(duplicates),
            std::end(duplicates),
            std::back_inserter(uniqueIndexes));
        musclesToSynthesise = uniqueIndexes;

        subject.getMusclesIndexFromMusclesList(indexesOfMusclesWithExcitationsToAdjust_, musclesToAdjust);
        subject.getMusclesIndexFromMusclesList(indexesOfMusclesWithExcitationsToPredict_, musclesToSynthesise);
        
        auto muscleNames(subject.getMuscleNames());
        for (auto m :musclesToSynthesise) {
            if (std::find(begin(muscleNames), end(muscleNames), m) == muscleNames.end())
                std::cout << "WARNING. MTU " << m << " is on the synthMTUs list, but does not exist in the subject.\n";
        }

        for (auto m : musclesToAdjust) {
            if (std::find(begin(muscleNames), end(muscleNames), m) == muscleNames.end())
                std::cout << "WARNING. MTU " << m << " is on the adjustMTUs list, but does not exist in the subject.\n";
        }

        
        for (auto m : muscleNames) {
            std::cout << m << " -> ";
            if (std::find(begin(musclesToSynthesise), end(musclesToSynthesise), m) != musclesToSynthesise.end())
                std::cout << " Synthetising\n";
            else if (std::find(begin(musclesToAdjust), end(musclesToAdjust), m) != musclesToAdjust.end())
                std::cout << " Adjusting\n";
            else
                std::cout << " Unchanged\n";
        }

        weightings_ = setup.weightings;
        std::cout << "Using alpha = " << weightings_.alpha << ", "
            << "beta = " << weightings_.beta << ", "
            << "gamma = " << weightings_.gamma << std::endl;
        
        //at the moment it's all the dofs, but we may want to set that through xml
        for (unsigned i = 0; i < subject.getNoDoF(); ++i)
            indexesOfTorquesToMinimise_.push_back(i);



        //get torques to optimise from setup

    }
}

#endif
