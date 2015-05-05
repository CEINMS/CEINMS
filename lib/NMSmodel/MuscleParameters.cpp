//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <limits>
#include "MuscleParameters.h"

static const double dblLow(std::numeric_limits<double>::lowest());

namespace ceinms {
    
    MuscleParameters::MuscleParameters() :
        c1_(std::numeric_limits<double>::lowest()), c2_(dblLow), shapeFactor_(dblLow),
        optimalFiberLength_(dblLow), pennationAngle_(dblLow),
        tendonSlackLength_(dblLow), maxIsometricForce_(dblLow),
        strengthCoefficient_(dblLow), emDelay_(dblLow)
    {

    }

    bool MuscleParameters::operator==(const MuscleParameters &other) const {

        return  c1_ == other.c1_                                    &&
                c2_ == other.c2_                                    &&
                shapeFactor_ == other.shapeFactor_                  &&
                optimalFiberLength_ == other.optimalFiberLength_    &&
                pennationAngle_ == other.pennationAngle_            &&
                tendonSlackLength_ == other.tendonSlackLength_      &&
                maxIsometricForce_ == other.maxIsometricForce_      &&
                strengthCoefficient_ == other.strengthCoefficient_  &&
                emDelay_ == other.emDelay_;
    }


    MuscleParameters::~MuscleParameters()
    {

    }
}