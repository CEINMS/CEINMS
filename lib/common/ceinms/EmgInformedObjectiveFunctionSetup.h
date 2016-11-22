#ifndef ceinms_EmgInformedObjectiveFunctionSetup_h
#define ceinms_EmgInformedObjectiveFunctionSetup_h

#include <vector>
#include <string>
#include "ceinms/HybridWeightings.h"

namespace ceinms {
    struct EmgInformedObjectiveFunctionSetup {

        HybridWeightings weightings;
        std::vector<std::string> musclesToSynthesise;
        std::vector<std::string> musclesToAdjust;

    };
}
#endif