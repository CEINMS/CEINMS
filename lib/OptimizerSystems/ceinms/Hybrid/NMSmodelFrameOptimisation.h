#ifndef ceinms_NMSmodelFrameOptimisation_h
#define ceinms_NMSmodelFrameOptimisation_h

#include <Simbody.h>
#include "ceinms/ArrayConverter.h"
#include "ceinms/Hybrid/EmgInformedSystem.h"
#include "ceinms/Hybrid/TrackTorqueAdjustAndPredictExcitations.h"
#include "ceinms/Hybrid/MuscleExcitationParameterInterpreter.h"
#include "ceinms/EmgInformedObjectiveFunctionSetup.h"
namespace ceinms {

    template<typename EmgInformedSystemT>
    class SimTKAdapter : public SimTK::OptimizerSystem{

    public:
        SimTKAdapter(EmgInformedSystemT& emgInformedSystem) :
            emgInformedSystem_(emgInformedSystem)
        {
            OptimizerSystem::setNumEqualityConstraints(0);
            OptimizerSystem::setNumInequalityConstraints(0);
            OptimizerSystem::setNumParameters(emgInformedSystem_.getNoParameters());
        }

        int objectiveFunc(const SimTK::Vector &coefficients, bool new_coefficients, SimTK::Real& f) const override {

            std::vector<double> coeff(ArrayConverter::fromSimTKVector(coefficients));
            emgInformedSystem_.setParameters(coeff);
            emgInformedSystem_.evaluate();
            f = emgInformedSystem_.getF();
            return(0);
        }
    private:
        EmgInformedSystemT& emgInformedSystem_;
    };


    template <typename NMSmodelT>
    class NMSmodelFrameOptimisation{

    public:
        NMSmodelFrameOptimisation(NMSmodelT& subject, const EmgInformedObjectiveFunctionSetup& setup);
        void setExternalTorques(const std::vector<double>& externalTorques);
        void optimise();
        //The ObjFun = w[0]*c[0] + w[1]*c[1] + ... + w[n]*c[n]
        //where w is the weight and c the component
        void getObjectiveFunctionComponentsAndWeightings(std::vector<double>& components, std::vector<double>& weights) const;
    private:
        using MyEmgInformedSystem = EmgInformedSystem<NMSmodelT, TrackTorqueAdjustAndPredictExcitations, MuscleExcitationParameterInterpreter<NMSmodelT>>;
        NMSmodelT& subject_;
        TrackTorqueAdjustAndPredictExcitations objFun_;
        MuscleExcitationParameterInterpreter<NMSmodelT> parameterInterpreter_;
        MyEmgInformedSystem emgInformed_;
        SimTKAdapter<MyEmgInformedSystem> simTkAdapter_;
    };

}
#include "ceinms/Hybrid/NMSmodelFrameOptimisation.cpp"
#endif