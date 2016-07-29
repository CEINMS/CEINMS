#ifndef ceinms_ComposedObjectiveFunction_h
#define ceinms_ComposedObjectiveFunction_h

#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include "ceinms/Result.h"
#include "ceinms/TrialData.h"
#include "ceinms/ObjectiveFunctionBase.h"

namespace ceinms {

    class ComposedObjectiveFunction : public ObjectiveFunctionBase {
    public :
        ComposedObjectiveFunction();
        ComposedObjectiveFunction(const std::vector<std::shared_ptr<ObjectiveFunctionBase>>& objectiveFunctions);
        void appendObjectiveFunction(std::shared_ptr<ObjectiveFunctionBase> objectiveFunction);
        void setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate) override;
        void setTrials(const std::vector<TrialData>& trials) override;
        void calculate(const std::vector<Result>& results) override;
        double getValue() const override;
        void printCosts() const override;
    private:
        std::vector<std::shared_ptr<ObjectiveFunctionBase>> objectiveFunctions_;
        double f_;
        std::vector<TrialData> trials_;
        std::vector<std::string> dofsToCalibrate_;
    };

}

#endif