#include "MTUutils.h"
#include <cmath>

namespace ceinms {
    namespace PennationAngle {

        double compute(double fibreLength, double optimalFibreLength, double pennationAngle) {

            double value(optimalFibreLength*std::sin(pennationAngle) / fibreLength);
            if (value <= 0.0)
                return .0;
            else if (value >= 1.0)
                return M_PI_2;
            return (std::asin(value));
        }
    }
}
