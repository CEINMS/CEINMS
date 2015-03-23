#ifndef Ceinms_NMSmodelSystem_h
#define Ceinms_NMSmodelSystem_h

#include "AbstractOptimizerSystem.h"

#include "ParametersInterpreter.h" //TODO: rename this class
#include "TrialData.h"
#include "BatchEvaluator.h"
#include <vector>

namespace CEINMS {
    template <typename NMSmodelT, typename ObjectiveFunctionT>
    class NMSmodelSystem : public Optimizers::AbstractOptimizerSystem  {
    public:
        NMSmodelSystem(NMSmodelT& subject, const std::vector<TrialData>& trialData, const ObjectiveFunctionT& objectiveFunction, const Parameter::ParameterSet& parameterSet);

        virtual ~NMSmodelSystem() {}
        void setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate);
        void evaluate();
        void setParameters(const std::vector<double>& x);
        std::vector<double> getParameters();
        double getF() const { return f_; }
        size_t getNoParameters() { return parameterInterpreter_.getNoParameters(); }
        void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const; 
 //       void setTrialData(const std::vector<TrialData>& trialData) { trialData_ = trialData; }
 //       void pushTrialData(const TrialData& trialData) { trialData_.emplace_back(trialData); }


    private:

        NMSmodelT& subject_;
        BatchEvaluator batchEvaluator_;
        ObjectiveFunctionT objectiveFunction_;
        ParametersInterpreter<NMSmodelT> parameterInterpreter_;
        std::vector<std::string> dofsToCalibrate_;
        double f_;
  //      std::vector<TrialData> trialData_; //I'd like this to be here.. but it ended up in BatchEvaluator.. which has the advantage to hide 
    };
}


//#include "NMSmodelSystem.cpp"
#endif