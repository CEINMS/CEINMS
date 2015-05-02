#include "MTUutils.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace CEINMS {

    namespace PennationAngle {

        double compute(double fibreLength, double optimalFibreLength, double pennationAngle) {

            double value(optimalFibreLength*sin(pennationAngle) / fibreLength);
            if (value <= 0.0)
                return .0;
            else if (value >= 1.0)
                return M_PI_2;
            return (asin(value));
        }
    }
}
