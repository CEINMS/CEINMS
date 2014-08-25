#ifndef NumCompare_h
#define NumCompare_h

#include <limits>
#include <math.h>
namespace NumCompare {

    bool equal(double a, double b) {

        //        return fabs(a - b) <= std::numeric_limits<double>::epsilon();
        return fabs(a - b) <= 0.0001;
    }

    bool lessEqual(double a, double b) {

        return equal(a, b) || a < b;
    }

}

#endif