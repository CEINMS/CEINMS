#ifndef ceinms_BarrierFunction_h
#define ceinms_BarrierFunction_h

#include <vector>
#include "ceinms/Barrier.h"
#include "ceinms/ObjectiveFunctionSetup.h"
#include "ceinms/Mapper.h"

namespace ceinms {
    class BarrierFunction {
    public:
        BarrierFunction();
        void initialize(const BarrierListSetup& setup, const std::vector<std::string>& namesFromInput);
        double getValue(const std::vector<double>& x) const;
    private:
        std::vector<Barrier> barriers_;
        std::vector<std::string> barrierTargetNames_;
        Mapper barrierMapper_;
        double f_;
    };
}


#endif