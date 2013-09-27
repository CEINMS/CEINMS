#include "MuscleParameters.h"
#include "float.h"

MuscleParameters::MuscleParameters():
c1_(-DBL_MAX), c2_(-DBL_MAX), shapeFactor_(-DBL_MAX),
optimalFiberLength_(-DBL_MAX), pennationAngle_(-DBL_MAX),
tendonSlackLength_(-DBL_MAX), maxIsometricForce_(-DBL_MAX),
strengthCoefficient_(-DBL_MAX), emDelay_(-DBL_MAX)
{

}

bool MuscleParameters::operator==(const MuscleParameters &other) const {

    return  c1_                  == other.c1_                   &&
            c2_                  == other.c2_                   &&
            shapeFactor_         == other .shapeFactor_         &&
            optimalFiberLength_  == other.optimalFiberLength_   &&
            pennationAngle_      == other.pennationAngle_       &&   
            tendonSlackLength_   == other.tendonSlackLength_    &&
            maxIsometricForce_   == other.maxIsometricForce_    &&
            strengthCoefficient_ == other.strengthCoefficient_  &&
            emDelay_             == other.emDelay_;
}


MuscleParameters::~MuscleParameters()
{

}
