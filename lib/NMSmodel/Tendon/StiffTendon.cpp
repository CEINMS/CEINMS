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
#include <cmath>


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
                          double maxContractionVelocity,
                          const CurveOffline& activeForceLengthCurve, 
                          const CurveOffline& passiveForceLengthCurve, 
                          const CurveOffline& forceVelocityCurve,
                          const CurveOffline& tendonForceStrainCurve):
optimalFibreLength_(optimalFibreLength),
pennationAngle_(pennationAngle),
tendonSlackLength_(tendonSlackLength)
{

}


void StiffTendon::setParametersToComputeForces(double optimalFibreLength,
                                               double pennationAngle,
                                               double tendonSlackLength,
                                               double percentageChange,
                                               double damping, 
                                               double maxIsometricForce, 
                                               double strengthCoefficient,
                                               double maxContractionVelocity) {
    
    optimalFibreLength_ = optimalFibreLength;
    pennationAngle_     = pennationAngle;
    tendonSlackLength_  = tendonSlackLength;
}


void StiffTendon::setMuscleTendonLength(double muscleTendonLength) {

    muscleTendonLength_ = muscleTendonLength;
}


void StiffTendon::setActivation(double activation) {
    
}


void StiffTendon::updateFibreLength() {
 
    double first = optimalFibreLength_ * sin( pennationAngle_);
    double second = muscleTendonLength_ - tendonSlackLength_;
    fibreLength_ = sqrt(first*first + second*second);     
}


void StiffTendon::resetState() {
    
    muscleTendonLength_ = 0;
    fibreLength_ = 0;

}


StiffTendon::~StiffTendon()
{
   
}
