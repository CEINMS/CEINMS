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
        std::vector<double> musclesToAdjustMaxExcitations;
        std::vector<std::string> dofsToTrack;
        std::vector<double> dofsToTrackMaxTorque;

    };
}
#endif