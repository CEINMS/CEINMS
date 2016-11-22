#ifndef ceinms_MuscleExcitationParameterInterpreter_h
#define ceinms_MuscleExcitationParameterInterpreter_h

#include "ceinms/EmgInformedObjectiveFunctionSetup.h"
#include "ceinms/Mapper.h"

namespace ceinms{

    template<typename NMSmodelT>
    class MuscleExcitationParameterInterpreter {

    public:
        using ParameterSet = EmgInformedObjectiveFunctionSetup;
        MuscleExcitationParameterInterpreter(NMSmodelT& subject, const ParameterSet& setup);
        void setParameters(const std::vector<double>& x);
        std::vector<double> getStartingParameters() const;
        std::vector<double> getParameters() const;
        std::size_t getNoParameters() const { return noParameters_; }
        void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const;
    private:
        std::size_t noParameters_;
        NMSmodelT& subject_;
        Mapper parametersToExcitations_;
        Mapper excitationsToParameters_;
        
    };
}
#include "ceinms/Hybrid/MuscleExcitationParameterInterpreter.cpp"
#endif