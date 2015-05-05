#ifndef ceinms_NMSmodelSystem_h
#define ceinms_NMSmodelSystem_h

#include "AbstractOptimizerSystem.h"

#include "ParametersInterpreter.h" //TODO: rename this class
#include "TrialData.h"
#include "BatchEvaluator.h"
#include <vector>

namespace ceinms {
    template <typename NMSmodelT, typename ObjectiveFunctionT>
    class NMSmodelSystem : public Optimizers::AbstractOptimizerSystem  {
    public:
        NMSmodelSystem(NMSmodelT& subject, const std::vector<TrialData>& trialData, const ObjectiveFunctionT& objectiveFunction, const Parameter::Set& parameterSet, const std::vector<std::string>& dofsToCalibrate);

        virtual ~NMSmodelSystem() {}
        //    void setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate);
        void evaluate();
        void setParameters(const std::vector<double>& x);
        std::vector<double> getParameters();
        double getF() const { return f_; }
        size_t getNoParameters() const { return nParameters_; }
        std::vector<double> getParameters() const { return parameterInterpreter_.getSubjectParameters(); }
        void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const;
        //       void setTrialData(const std::vector<TrialData>& trialData) { trialData_ = trialData; }
        //       void pushTrialData(const TrialData& trialData) { trialData_.emplace_back(trialData); }


    private:

        size_t nParameters_;
        std::vector<NMSmodelT> mockSubjects_;
        NMSmodelT& subject_;
        BatchEvaluator batchEvaluator_;
        ObjectiveFunctionT objectiveFunction_;
        ParametersInterpreter<NMSmodelT> parameterInterpreter_;
        std::vector<std::string> dofsToCalibrate_;
        double f_;
        //      std::vector<TrialData> trialData_; //I'd like this to be here.. but it ended up in BatchEvaluator.. which has the advantage to hide 
    };
}


#include "NMSmodelSystem.cpp"
#endif