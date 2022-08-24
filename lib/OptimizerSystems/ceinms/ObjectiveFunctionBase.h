#ifndef ceinms_ObjectiveFunctionBase_h
#define ceinms_ObjectiveFunctionBase_h

#include <vector>
#include <string>

#include "ceinms/TrialData.h"
#include "ceinms/Result.h"

namespace ceinms {

    class ObjectiveFunctionBase {
    public:
        virtual void setTrials(const std::vector<TrialData>& trials) = 0;
        virtual void calculate(const std::vector<Result>& results) = 0;
        virtual double getValue() const = 0;
        virtual void printCosts() const = 0;
        virtual bool calculateOpenSimData() const {
            return false;
        }
    };
}

#endif