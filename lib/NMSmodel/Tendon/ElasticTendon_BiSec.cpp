//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <cmath>
#include <vector>
using std::vector;
#include "float.h"
#include <iostream>
using std::cout;
using std::endl;
#include "ElasticTendon_BiSec.h"
#include <functional>
//#include <boost/concept_check.hpp>
#include "WDBsolver.h"
#include "../MTUutils.h" //don't know why it doesn't show up without the parent dir..

//#define DEEP_DEBUG
//#define DEBUG

// template <typename T> int sgn(T val) {
//     return (T(0) < val) - (val < T(0));
// }



ElasticTendon_BiSec::ElasticTendon_BiSec():
optimalFibreLength_(.0),
pennationAngle_(.0),
tendonSlackLength_(.0),
percentageChange_(.0),
damping_(.0),
maxIsometricForce_(.0), 
strengthCoefficient_(.0),
muscleTendonLength_(0.0),
fibreLength_(0.0),
activation_(0.0),
maxContractionVelocity_(0.0),
id_(""),
tolerance_(.000001)
{ }


ElasticTendon_BiSec::ElasticTendon_BiSec(std::string id):
optimalFibreLength_(.0),
pennationAngle_(.0),
tendonSlackLength_(.0),
percentageChange_(.0),
damping_(.0),
maxIsometricForce_(.0), 
strengthCoefficient_(.0),
muscleTendonLength_(0.0),
fibreLength_(0.0),
activation_(0.0),
maxContractionVelocity_(0.0),
tolerance_(.000001),
id_(id) { }


ElasticTendon_BiSec::ElasticTendon_BiSec (double optimalFibreLength, 
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
tendonSlackLength_(tendonSlackLength),
percentageChange_(percentageChange),
damping_(damping),
maxIsometricForce_(maxIsometricForce), 
strengthCoefficient_(strengthCoefficient),
activeForceLengthCurve_(activeForceLengthCurve),
passiveForceLengthCurve_(passiveForceLengthCurve),
forceVelocityCurve_(forceVelocityCurve),
tendonForceStrainCurve_(tendonForceStrainCurve),
maxContractionVelocity_(maxContractionVelocity),
muscleTendonLength_(0.0),
fibreLength_(0.0),
activation_(0.0),
id_(""),
tolerance_(.000001)
{   }


ElasticTendon_BiSec::ElasticTendon_BiSec ( const ElasticTendon_BiSec& orig ) {
    
    cout << "ElasticTendon_BiSec copy constructor. EXIT\n";
    exit(EXIT_FAILURE);
}


ElasticTendon_BiSec& ElasticTendon_BiSec::operator= ( const ElasticTendon_BiSec& orig ) {

    optimalFibreLength_      = orig.optimalFibreLength_;
    pennationAngle_          = orig.pennationAngle_;
    tendonSlackLength_       = orig.tendonSlackLength_;
    percentageChange_        = orig.percentageChange_;
    damping_                 = orig.damping_;
    maxIsometricForce_       = orig.maxIsometricForce_;
    strengthCoefficient_     = orig.strengthCoefficient_;
    maxContractionVelocity_ = orig.maxContractionVelocity_,
    activeForceLengthCurve_  = orig.activeForceLengthCurve_;
    passiveForceLengthCurve_ = orig.passiveForceLengthCurve_;
    forceVelocityCurve_      = orig.forceVelocityCurve_;
    tendonForceStrainCurve_  = orig.tendonForceStrainCurve_;
    
    muscleTendonLength_      = orig.muscleTendonLength_;
    fibreLength_             = orig.fibreLength_;
    activation_              = orig.activation_;
    id_                      = orig.id_;
    tolerance_               = orig.tolerance_;
    return *this;
}


void ElasticTendon_BiSec::setParametersToComputeForces(double optimalFiberLength,
                                                 double pennationAngle,
                                                 double tendonSlackLength,
                                                 double percentageChange,
                                                 double damping, 
                                                 double maxIsometricForce, 
                                                 double strengthCoefficient,
                                                 double maxContractionVelocity) {
 
    optimalFibreLength_  = optimalFiberLength;
    pennationAngle_      = pennationAngle;
    tendonSlackLength_   = tendonSlackLength;
    percentageChange_    = percentageChange;
    damping_             = damping;
    maxIsometricForce_   = maxIsometricForce;
    strengthCoefficient_ = strengthCoefficient;
    maxContractionVelocity_ = maxContractionVelocity;
}


void ElasticTendon_BiSec::setMuscleTendonLength(double muscleTendonLength) {

    muscleTendonLength_ = muscleTendonLength;
}


void ElasticTendon_BiSec::setActivation(double activation) {

    activation_ = activation;
}



void ElasticTendon_BiSec::updateFibreLength() {

    const unsigned nIter = 100;
    tendonPenalty_ = .0;
    fibreLength_ = estimateFiberLengthBiSec(tolerance_, nIter);
    fibreLengthTrace_.addPoint(time_, fibreLength_);
}


void ElasticTendon_BiSec::pushState() {
    
}


void ElasticTendon_BiSec::setCurves(const CurveOffline& activeForceLengthCurve, 
                                          const CurveOffline& passiveForceLengthCurve, 
                                          const CurveOffline& forceVelocityCurve,
                                          const CurveOffline& tendonForceStrainCurve) { 
                                  
    activeForceLengthCurve_  = activeForceLengthCurve;
    passiveForceLengthCurve_ = passiveForceLengthCurve;
    forceVelocityCurve_      = forceVelocityCurve;
    tendonForceStrainCurve_  = tendonForceStrainCurve;
}

void ElasticTendon_BiSec::setTolerance(double tolerance){
    if (tolerance > 0.0)
        tolerance_ = tolerance;
}

void ElasticTendon_BiSec::setStrengthCoefficient (double strengthCoefficient) {
    
    strengthCoefficient_ = strengthCoefficient;
    resetState();
}


void ElasticTendon_BiSec::setTendonSlackLength (double tendonSlackLength) {
    
    tendonSlackLength_ = tendonSlackLength;
    resetState();
}


void ElasticTendon_BiSec::setPennationAngle(double pennationAngle) {

    pennationAngle_ = pennationAngle;
    resetState();
}


void ElasticTendon_BiSec::setMaxIsometricForce(double maxIsometricForce) {

    maxIsometricForce_ = maxIsometricForce;
    resetState();
}


void ElasticTendon_BiSec::setOptimalFibreLength(double optimalFibreLength) {

    optimalFibreLength_ = optimalFibreLength;
    resetState();
}


void ElasticTendon_BiSec::setMaxContractionVelocity(double maxContractionVelocity) {

    maxContractionVelocity_ = maxContractionVelocity;
    resetState();
}

void ElasticTendon_BiSec::resetState() {

    muscleTendonLength_ = 0.0;
    fibreLength_ = 0.0;
    activation_ = 0.0;
    fibreLengthTrace_.reset();
}


double ElasticTendon_BiSec::estimateFiberLengthBiSec(double tol, unsigned maxIterations) {

 //   cout << "------------------\n";
 //   cout << "Fibre Length for " << id_ << endl; 
       
#ifdef DEEP_DEBUG
    cout << "Start DD\n";
    const unsigned nSteps = 100;
    double incr = 2.0*optimalFibreLength_/nSteps;
    double fl = 0;//optimalFibreLength_*0.5;
    cout << "tendon Force\n";
    for(unsigned s = 0; s < nSteps; ++s) {
        fl += incr;
        //cout << fl << " " << evaluateForceError(fl) << endl;
        cout << fl << " " << computeTendonForce(fl) << endl;    
    }
    cout << "muscle Force\n";
    fl = 0;
     for(unsigned s = 0; s < nSteps; ++s) {
        fl += incr;
        //cout << fl << " " << evaluateForceError(fl) << endl;
        cout << fl << " " << computeMuscleForce(fl) << endl;    
    }
    cout << "End DD\n";
#endif
    
    bool runCondition = true;
    unsigned nIter = 0;

    double optimalFibreLengthAtT = optimalFibreLength_ * (percentageChange_ *
                                   (1.0 - activation_) + 1 ); 
          
    double minFibreLength = 0.2*optimalFibreLength_;
    double maxFibreLength = 2*optimalFibreLength_;
    double currentFibreLength = optimalFibreLength_;
#ifdef DEBUG
    cout << "Error @ minFibreLength " << evaluateForceError(minFibreLength) << endl;
    cout << "Error @ maxFibreLength " << evaluateForceError(maxFibreLength) << endl;
#endif
   
    try {  
        currentFibreLength = wdbSolve(*this, minFibreLength, maxFibreLength, tol);
    //     currentFibreLength = rtSafe(*this, minFibreLength, maxFibreLength, tol);
        
    } catch (...) {

    //     cout << "Exception: cannot solve " << id_ << " setting currentFibreLength=optimalFibreLength\nSwitching to stiff tendon\n";
        currentFibreLength = getFibreLengthStiff();
        tendonPenalty_+= 100;
    }

    return currentFibreLength;


}


double ElasticTendon_BiSec::operator()(double fl) {

    return evaluateForceError(fl);
}
 

double ElasticTendon_BiSec::evaluateForceError(double fiberLength) {
    
    double tendonForce = computeTendonForce(fiberLength);
    double muscleForce = computeMuscleForce(fiberLength);
 //   cout << "tendonForce " << tendonForce << endl;
 //   cout << "muscleForce " << muscleForce << endl;
    return (tendonForce - muscleForce);
}


double ElasticTendon_BiSec::computeTendonForce(double fibreLength) {
    
    double pennationAngleAtT = CEINMS::PennationAngle::compute(fibreLength, optimalFibreLength_, pennationAngle_);
    double tendonLength = muscleTendonLength_ - fibreLength*cos(pennationAngleAtT);  
    double tendonStrain = (tendonLength - tendonSlackLength_)/tendonSlackLength_;
    double tendonForce = strengthCoefficient_*maxIsometricForce_*
                          tendonForceStrainCurve_.getValue(tendonStrain);
                         
  /*  cout << "muscleTendonLength_ " << muscleTendonLength_ << endl;
    cout << "tendonForce " << tendonForce << endl;
    cout << "tendonLength " << tendonLength << endl;
    cout << "tendonStrain " << tendonStrain << endl;
    cout << "pennationAngleAtT " << pennationAngleAtT << endl;
    cout << "fibreLength " << fibreLength << endl;*/
//    cout << "pennationAngleAtT " << pennationAngleAtT << endl;
//    cout << "pennationAngle_ " << pennationAngle_ << endl;
    return tendonForce;
}


double ElasticTendon_BiSec::computeMuscleForce(double fibreLength) {
    
    double optimalFiberLengthAtT = optimalFibreLength_ * (percentageChange_ *
                                   (1.0 - activation_) + 1 ); 
          
    fibreLengthTrace_.addPoint(time_, fibreLength);
    double normFiberLength   = fibreLength / optimalFibreLength_;
    double normFiberLengthAtT   = fibreLength / optimalFiberLengthAtT;

    double normFiberVelocity = fibreLengthTrace_.getFirstDerivative(time_)/optimalFibreLength_;
    if (normFiberVelocity > maxContractionVelocity_)
        normFiberVelocity = maxContractionVelocity_;
    if (normFiberVelocity < -maxContractionVelocity_)
        normFiberVelocity = -maxContractionVelocity_;
    normFiberVelocity /= maxContractionVelocity_;

    double fv = forceVelocityCurve_.getValue(normFiberVelocity);
    double fp = passiveForceLengthCurve_.getValue(normFiberLength);
    double fa = activeForceLengthCurve_.getValue(normFiberLengthAtT);
    double pennationAngleAtT = CEINMS::PennationAngle::compute(fibreLength, optimalFibreLength_, pennationAngle_);

    
    double muscleForce = maxIsometricForce_ * strengthCoefficient_ *
           (fa * fv * activation_ + fp + damping_ * normFiberVelocity)* 
           cos(pennationAngleAtT);

    fibreLengthTrace_.removeLastPointNoUpdate();
    
//    cout << "muscleForce " << muscleForce << endl;
    return muscleForce;
}


double ElasticTendon_BiSec::getFibreLengthStiff() const {
 
    double first = optimalFibreLength_ * sin( pennationAngle_);
    double second = muscleTendonLength_ - tendonSlackLength_;
    return sqrt(first*first + second*second);     
}

/*
void ElasticTendon_BiSec::setMuscleTendonLength(double muscleTendonLength, double activation, double time) {
    
    muscleTendonLength_ = muscleTendonLength;
    activation_ = activation;
    updateFibreLength();

}
*/




