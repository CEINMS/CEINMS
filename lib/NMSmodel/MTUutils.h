#ifndef ceinms_MTUutils_h
#define ceinms_MTUutils_h

#define _USE_MATH_DEFINES
#include <math.h>

namespace CEINMS {

    namespace PennationAngle {

        double compute(double fibreLength, double optimalFibreLength, double pennationAngle);
    }
}

inline double radians(double d) {
    return d * M_PI / 180;
}
#endif