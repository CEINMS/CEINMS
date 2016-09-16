#ifndef ceinms_EmgInformedSystem_h
#define ceinms_EmgInformedSystem_h

#include <vector>
#include "ceinms/AbstractOptimizerSystem.h"

namespace ceinms {
    
    template<typename NMSmodelT, typename ObjectiveFunctionT, typename ParametersInterpreterT>
    class EmgInformedSystem : public Optimizers::AbstractOptimizerSystem {

    public:
        EmgInformedSystem(NMSmodelT& subject, const ObjectiveFunctionT& objectiveFunction, const typename ParametersInterpreterT::ParameterSet& parameterSet);
        void evaluate() override;
        void setParameters(const std::vector<double>& x) override;
        void setExternalTorque(const std::vector<double>& externalTorques);
        void setMeasuredExcitations(const std::vector<double>& measuredExcitations);
        std::vector<double> getParameters() const;
        double getF() const override;
        std::size_t getNoParameters() const override { return nParameters_; }
    private:
        std::size_t nParameters_;
        ObjectiveFunctionT objectiveFunction_;
        ParametersInterpreterT parametersInterpreter_;
        NMSmodelT& subject_;
        double f_;
    };
    
}

#include "ceinms/Hybrid/EmgInformedSystem.cpp"

#endif