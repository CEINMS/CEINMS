//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ElasticTendon_BiSec_h
#define ElasticTendon_BiSec_h
#include <string>
#include "Curve.h"

//template <typename CurveMode::Mode mode>
class ElasticTendon_BiSec {

public:
    typedef Curve<CurveMode::Offline> CurveOffline;
    ElasticTendon_BiSec();
    ElasticTendon_BiSec(std::string id);
    ElasticTendon_BiSec( double optimalFibreLength,
                   double pennationAngle,
                   double tendonSlackLength,
                   double percentageChange,
                   double damping,
                   double maxIsometricForce, 
                   double strengthCoefficient,
                   const CurveOffline& activeForceLengthCurve,
                   const CurveOffline& passiveForceLengthCurve, 
                   const CurveOffline& forceVelocityCurve,
                   const CurveOffline& tendonForceStrainCurve);
    virtual ~ElasticTendon_BiSec() {}
    double operator()(double fl);
    ElasticTendon_BiSec(const ElasticTendon_BiSec& orig);
    ElasticTendon_BiSec& operator=(const ElasticTendon_BiSec& orig);
    
    void setParametersToComputeForces(double optimalFibreLength,
                                      double pennationAngle,
                                      double tendonSlackLength,
                                      double percentageChange,
                                      double damping, 
                                      double maxIsometricForce, 
                                      double strengthCoefficient); 

    void setTime(const double& time) {    time_ = time; }
    void setMuscleTendonLength(double muscleTendonLength);
    void setActivation(double activation);
    void updateFibreLength();
    double getPenalty() const { return tendonPenalty_;}

    double getFibreLength() { return fibreLength_;}
    void setStrengthCoefficient(double strengthCoefficient);
    void setTendonSlackLength(double tendonSlackLength);
    void setOptimalFibreLength(double optimalFibreLength) { optimalFibreLength_ = optimalFibreLength; }
    void setCurves(const CurveOffline& activeForceLengthCurve, 
                   const CurveOffline& passiveForceLengthCurve, 
                   const CurveOffline& forceVelocityCurve,
                   const CurveOffline& tendonForceStrainCurve);
                   
    void pushState();
    void resetState();
    
    
    /*
    
    void setMuscleTendonLength(double muscleTendonLength, double activation, double time);
    void updateFiberLengthUsingNewActivation(double activation, double time);
    double getFiberLength() { return fibreLength_;}
    void setStrengthCoefficient(double strengthCoefficient);
    void setTendonSlackLength(double tendonSlackLength);
    void setCurves(const CurveOffline& activeForceLengthCurve, 
                   const CurveOffline& passiveForceLengthCurve, 
                   const CurveOffline& forceVelocityCurve);
                   
    void resetState();
    */
private:
    double evaluateForceError(double fibreLength);
    double estimateFiberLengthBiSec(double tol, unsigned maxIterations);
    double computeMuscleForce(double fibreLength);
    double computeTendonForce(double fibreLength);
    double getFibreLengthStiff() const;
    
    double optimalFibreLength_;
    double pennationAngle_;
    double tendonSlackLength_;
    double percentageChange_;
    double damping_;
    double maxIsometricForce_; 
    double strengthCoefficient_;
    CurveOffline activeForceLengthCurve_;
    CurveOffline passiveForceLengthCurve_;
    CurveOffline forceVelocityCurve_;
    CurveOffline tendonForceStrainCurve_;

    Curve<CurveMode::Online>  fibreLengthTrace_;
    double fibreLength_;
    double muscleTendonLength_;
    double activation_;
    double tendonPenalty_;
    double time_;
    std::string id_;
    
};

#endif