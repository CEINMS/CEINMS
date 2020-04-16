#ifndef ceinms_CompositeObjectiveFunction_h
#define ceinms_CompositeObjectiveFunction_h

#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include "ceinms/Result.h"
#include "ceinms/TrialData.h"
#include "ceinms/ObjectiveFunctionBase.h"
#include "ceinms/ObjectiveFunctionSetup.h"

namespace ceinms {

    class CompositeObjectiveFunction : public ObjectiveFunctionBase {
    public :
        CompositeObjectiveFunction();
        CompositeObjectiveFunction(const std::vector<std::shared_ptr<ObjectiveFunctionBase>>& objectiveFunctions);
        void appendObjectiveFunction(std::shared_ptr<ObjectiveFunctionBase> objectiveFunction);
        void setTrials(const std::vector<TrialData>& trials) override;
        void calculate(const std::vector<Result>& results) override;
        double getValue() const override;
        void printCosts() const override;
        bool calculateOpenSimData() const override; 
        static CompositeObjectiveFunction makeComposite(
            ObjectiveFunctionSetupList objectiveFunctions);
    private:
        std::vector<std::shared_ptr<ObjectiveFunctionBase>> objectiveFunctions_;
        double f_;
        std::vector<TrialData> trials_;
        std::vector<std::string> dofsToCalibrate_;
    };

}

#endif