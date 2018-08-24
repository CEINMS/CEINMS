#ifndef ceinms_TrackTorqueAdjustAndPredictExcitations_h
#define ceinms_TrackTorqueAdjustAndPredictExcitations_h

#include <vector>
#include <algorithm>
#include <iterator>

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
        std::vector<double> externalTorqueErrors_;
        std::vector<double> measuredExcitations_;
        std::vector<double> measuredExcitationErrors_;
        unsigned nDofs_; //N dofs to track
        unsigned nSynth_; //N excitations to synthesis
        unsigned nAdj_; //N excitations to adjust

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
        

        //Get Dofs to track from setup
        std::cout << "Joint moments\n";
        std::vector<std::string> dofNames(subject.getDoFNames());
        for (const auto& d : dofNames) {
            if (std::find(begin(setup.dofsToTrack), end(setup.dofsToTrack), d) 
                == setup.dofsToTrack.end()) {
                std::cout << d << " -> Unchanged\n";
            }
            else {
                int dist = std::distance(begin(dofNames), std::find(begin(dofNames), end(dofNames), d));
                if (dist >= 0) {
                    indexesOfTorquesToMinimise_.push_back(static_cast<unsigned>(dist));
                    std::cout << d << " -> Tracking\n";
                }
            }
        }

        std::cout << "Muscle excitations\n";
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


        nDofs_ = std::max<unsigned>(1, indexesOfTorquesToMinimise_.size());
        nSynth_ = std::max<unsigned>(1, indexesOfMusclesWithExcitationsToPredict_.size()+ indexesOfMusclesWithExcitationsToAdjust_.size());
        nAdj_ = std::max<unsigned>(1, indexesOfMusclesWithExcitationsToAdjust_.size());
    }
}

#endif
