//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani, Massimo Sartori - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//         massimo.sartori@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "StiffTendon.h"
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

inline double radians (double d) {
return d * M_PI / 180;
}

inline double degrees (double r) {
return r * 180/ M_PI;
}

StiffTendon::StiffTendon()
{

}

StiffTendon::StiffTendon(std::string id):
id_(id)

{

}

StiffTendon::StiffTendon (double optimalFibreLength, 
                          double pennationAngle, 
                          double tendonSlackLength, 
                          double percentageChange, 
                          double damping, 
                          double maxIsometricForce, 
                          double strengthCoefficient, 
                          const CurveOffline& activeForceLengthCurve, 
                          const CurveOffline& passiveForceLengthCurve, 
                          const CurveOffline& forceVelocityCurve,
                          const CurveOffline& tendonForceStrainCurve):
optimalFibreLength_(optimalFibreLength),
pennationAngle_(pennationAngle),
tendonSlackLength_(tendonSlackLength),
percentageChange_(percentageChange)                         
{

}


void StiffTendon::setParametersToComputeForces(double optimalFibreLength,
                                               double pennationAngle,
                                               double tendonSlackLength,
                                               double percentageChange,
                                               double damping, 
                                               double maxIsometricForce, 
                                               double strengthCoefficient) {
    
    optimalFibreLength_ = optimalFibreLength;
    pennationAngle_     = pennationAngle;
    tendonSlackLength_  = tendonSlackLength;
    percentageChange_   = percentageChange;
    
}


void StiffTendon::setMuscleTendonLength(double muscleTendonLength) {

    muscleTendonLength_ = muscleTendonLength;
}


void StiffTendon::setActivation(double activation) {
    
    activation_ = activation;
}


void StiffTendon::updateFibreLength() {
 
    double first = optimalFibreLength_ * sin( radians(pennationAngle_));
    double second = muscleTendonLength_ - tendonSlackLength_;
    fibreLength_ = sqrt(first*first + second*second);     
}


void StiffTendon::resetState() {
    
    activation_ = 0;
    muscleTendonLength_ = 0;
    fibreLength_ = 0;

}


StiffTendon::~StiffTendon()
{
   
}
