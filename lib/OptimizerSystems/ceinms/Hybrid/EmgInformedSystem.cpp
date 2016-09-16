#include <limits>

namespace ceinms {

    template<
        typename NMSmodelT,
        typename ObjectiveFunctionT,
        typename ParametersInterpreterT
    >
    EmgInformedSystem<NMSmodelT, ObjectiveFunctionT, ParametersInterpreterT>::EmgInformedSystem(
        NMSmodelT& subject,
        const ObjectiveFunctionT& objectiveFunction,
        const typename ParametersInterpreterT::ParameterSet& parameterSet) :
    subject_(subject),
    objectiveFunction_(objectiveFunction),
    parametersInterpreter_(subject, parameterSet),
    f_(std::numeric_limits<double>::max()) 
    
    {
        nParameters_ = parametersInterpreter_.getNoParameters();
        /*add objective function initialisations*/
    
    }
   
    template<
        typename NMSmodelT,
        typename ObjectiveFunctionT,
        typename ParametersInterpreterT
    >
    void EmgInformedSystem<NMSmodelT, ObjectiveFunctionT, ParametersInterpreterT>::setExternalTorque(const std::vector<double>& externalTorques) {

        objectiveFunction_.setExternalTorques(externalTorques);
    }
    
    template<
        typename NMSmodelT,
        typename ObjectiveFunctionT,
        typename ParametersInterpreterT
    >
    void EmgInformedSystem<NMSmodelT, ObjectiveFunctionT, ParametersInterpreterT>::setMeasuredExcitations(const std::vector<double>& measuredExcitations) {

        objectiveFunction_.setMeasuredExcitations(measuredExcitations);

    }

    template<
        typename NMSmodelT,
        typename ObjectiveFunctionT,
        typename ParametersInterpreterT
    >
    void EmgInformedSystem<NMSmodelT, ObjectiveFunctionT, ParametersInterpreterT>::evaluate() {
        
        std::vector<double> predictedTorques, predictedExcitations;
        subject_.getTorques(predictedTorques);
        subject_.getEmgs(predictedExcitations);
        objectiveFunction_.calculate(predictedTorques, predictedExcitations);
        f_ = objectiveFunction_.getValue();
    }
    
    template<
        typename NMSmodelT,
        typename ObjectiveFunctionT,
        typename ParametersInterpreterT
    >
    void EmgInformedSystem<NMSmodelT, ObjectiveFunctionT, ParametersInterpreterT>::setParameters(const std::vector<double>& x) {

        parametersInterpreter_.setParameters(x);
    }

    template<
        typename NMSmodelT,
        typename ObjectiveFunctionT,
        typename ParametersInterpreterT
    >
    std::vector<double> EmgInformedSystem<NMSmodelT, ObjectiveFunctionT, ParametersInterpreterT>::getParameters() const {

        return parametersInterpreter_.getParameters();
    }


    template<
        typename NMSmodelT,
        typename ObjectiveFunctionT,
        typename ParametersInterpreterT
    >
    double EmgInformedSystem<NMSmodelT, ObjectiveFunctionT, ParametersInterpreterT>::getF() const {

        return f_;
    }


}