#include <vector>
#include <string>
using std::string;
using std::vector;
#include <algorithm>
using std::begin;
using std::end;

namespace ceinms {
    
    template<typename NMSmodelT>
    MuscleExcitationParameterInterpreter<NMSmodelT>::MuscleExcitationParameterInterpreter(
        NMSmodelT& subject, 
        const EmgInformedObjectiveFunctionSetup& setup) :
    subject_(subject) {

        vector<string> musclesToOptimise(setup.musclesToAdjust);
        musclesToOptimise.insert(end(musclesToOptimise), begin(setup.musclesToSynthesise), end(setup.musclesToSynthesise));
        excitationsToParameters_.setNames(subject_.getMuscleNames(), musclesToOptimise);
        parametersToExcitations_.setNames(musclesToOptimise, subject_.getMuscleNames());
        noParameters_ = musclesToOptimise.size();

    }

    template<typename NMSmodelT>
    void MuscleExcitationParameterInterpreter<NMSmodelT>::setParameters(const std::vector<double>& x) {
    

        vector<double> excitationsT1;
        subject_.getEmgs(excitationsT1); 
        auto excitations(parametersToExcitations_.map(x, excitationsT1));
        subject_.setEmgs(excitations);
        vector<unsigned> changedExcitations;
        for (unsigned i(0); i < excitations.size(); ++i) {
            if (excitations.at(i) != excitationsT1.at(i))
                changedExcitations.push_back(i);
        }
        subject_.updateState_HYBRID(changedExcitations);
    }
  
    template<typename NMSmodelT>
    std::vector<double> MuscleExcitationParameterInterpreter<NMSmodelT>::getParameters() const {
    
        vector<double> excitations;
        subject_.getEmgs(excitations);
        auto x(excitationsToParameters_.map(excitations));
        return x;    
    }

    template<typename NMSmodelT>
    std::vector<double> MuscleExcitationParameterInterpreter<NMSmodelT>::getStartingParameters() const {

        vector<double> excitations;
        subject_.getPastEmgs(excitations);
        auto x(excitationsToParameters_.map(excitations));
        return x;
    }

    template<typename NMSmodelT>
    void MuscleExcitationParameterInterpreter<NMSmodelT>::getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const {

        upperBounds = std::vector<double>(noParameters_, 0.999);
        lowerBounds = std::vector<double>(noParameters_, 0.001);
    }


}