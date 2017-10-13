#ifndef ceinms_PenaltyObjectiveFunction_h
#define ceinms_PenaltyObjectiveFunction_h

#include <vector>
#include "ceinms/Barrier.h"
#include "ceinms/ObjectiveFunctionSetup.h"
#include "ceinms/ObjectiveFunctionBase.h"
#include "ceinms/Result.h"
#include "ceinms/TrialData.h"

namespace ceinms {
    class PenaltyObjectiveFunction : public ObjectiveFunctionBase {
    public:
        PenaltyObjectiveFunction(const PenaltyObjectiveFunctionSetup& setup);
        void setTrials(const std::vector<TrialData>& trials) {/*blank*/} 
        void calculate(const std::vector<Result>& results);
        double getValue() const;
        void printCosts() const;
    private:
        double calculatePenalty(const DataTable<double>& data);
        double calculatePenalty(const DataTable<SimTK::Vec3>& data);
        Barrier barrier_;
        PenaltyObjectiveFunctionSetup setup_;
        double f_;
    };
}

#endif