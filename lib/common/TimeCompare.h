#ifndef ceinms_TimeCompare_h
#define ceinms_TimeCompare_h

#include <limits>
#include <cmath>
#define COMPARE_TOLERANCE 0.0001

//TODO: make a proper Time class
namespace ceinms {
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
}
#endif