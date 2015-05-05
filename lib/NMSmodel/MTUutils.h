#ifndef ceinms_MTUutils_h
#define ceinms_MTUutils_h

#include <cmath>

namespace CEINMS {

    namespace PennationAngle {

        double compute(double fibreLength, double optimalFibreLength, double pennationAngle);
    }
}

inline double radians(double d) {
    return d * M_PI / 180;
}
#endif