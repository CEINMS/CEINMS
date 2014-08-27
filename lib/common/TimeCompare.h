#ifndef TimeCompare_h
#define TimeCompare_h

#include <limits>
#include <math.h>
#define COMPARE_TOLERANCE 0.0001
namespace TimeCompare {

    bool equal(double a, double b) {

        //        return fabs(a - b) <= std::numeric_limits<double>::epsilon();
        return fabs(a - b) <= COMPARE_TOLERANCE;
    }

    bool lessEqual(double a, double b) {

        return equal(a, b) || a < b;
    }


    bool less(double a, double b) {
        // a has to be less than b, and not in the tolerance range
        return !equal(a, b) && a < b;
    }

}

#endif