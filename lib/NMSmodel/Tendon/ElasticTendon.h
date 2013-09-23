#ifndef ElasticTendon_h
#define ElasticTendon_h
#include "Curve.h"
#include <string>


// class for the initial fiber legth computation. #Revised#

class LDFM { 

public:
    typedef Curve<CurveMode::Offline> CurveOffline;
    LDFM()  {}  
    LDFM( double optimalFiberLength,
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
    virtual ~LDFM() {}
    
    
    void   setInitConditions(double muscleTendonLength, double muscleTendonVelocity, double activation);
    double estimateFiberLength();
    double oldEstimation(double muscleTendonLength, double muscleTendonVelocity, double activation);

private:
    void   updateInitConditions();
    void computeMuscleTendonForce();
    double computePennationAngle() const;
 
    
    //** parameters from MTU
    double optimalFiberLength_;
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
    //**
    
    
    //** initial condition parameters
    double activation_;
    double muscleTendonLength_;
    double muscleTendonVelocity_;
    //**
    
    
    //** variables used for the LDFM method
    double muscleTendonForce_;
    double fiberLength_; 
    double fiberVelocity_; 
    double fiberStiffness_;
    double tendonLength_; 
    double tendonVelocity_; 
    double tendonStiffness_;
    double optimalFiberLengthAtT_;
    //**
};


template <CurveMode::Mode mode>
class ElasticTendon {
    
public:
    typedef Curve<CurveMode::Offline> CurveOffline;
    typedef Curve<mode> CurveT;
    ElasticTendon();
    ElasticTendon(std::string id_);
    ElasticTendon(double optimalFiberLength,
                  double pennationAngle,
                  double tendonSlackLength,
                  double percentageChange,
                  double damping,
                  double maxIsometricForce, 
                  double strengthCoefficient,
                  const CurveOffline& activeForceLengthCurve,
                  const CurveOffline& passiveForceLengthCurve, 
                  const CurveOffline& forceVelocityCurve,
                  const CurveOffline& tendonForceStrainCurve
                 );
    virtual ~ElasticTendon();

    ElasticTendon(const ElasticTendon& orig);
    ElasticTendon& operator=(const ElasticTendon& orig);
    void operator() (double x, const std::vector<double> &y, std::vector<double> &dydx);
    void setParametersToComputeForces(double optimalFiberLength,
                                      double pennationAngle,
                                      double tendonSlackLength,
                                      double percentageChange,
                                      double damping, 
                                      double maxIsometricForce, 
                                      double strengthCoefficient); 

    void setMuscleTendonLength(double muscleTendonLength, double activation, double time);
    void updateFiberLengthUsingNewActivation(double activation, double time);
    double getFiberLength() const { return fiberLength_;}
    void setStrengthCoefficient(double strengthCoefficient);
    void setTendonSlackLength(double tendonSlackLength);
    void setCurves(const CurveOffline& activeForceLengthCurve, 
                   const CurveOffline& passiveForceLengthCurve, 
                   const CurveOffline& forceVelocityCurve,
                   const CurveOffline& tendonForceStrainCurve);
   
    void resetState();
    
private:
    double getInitialFiberLength(double time);
    double computeFiberVelocityAtT(double muscleTendonLengthAtT, double activationAtT, double muscleTendonVelocityAtT, double fiberLength, double lmtTimescale);
    double computeNormFiberVelocity(double activation, double activeForce, double velocityDependentForce); 
    
    double getFiberLengthRKF();
    
    //** Parameters from the MTU 
    double optimalFiberLength_;
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
    CurveT muscleTendonLengthTrace_;
    //**
    
    double testTime_;
//    Curve muscleTendonLengthTrace_; //verificare se questa serve in altre parti del tendine elastico
    double muscleTendonLength_;     //questa va passato dalla classe muscle -> vedere se si puo' evitare di farlo membre
    double muscleTendonLengthT1_;
    double muscleTendonVelocity_;   //questo viene derivato dall lunghezza mt -> come sopra
    double muscleTendonVelocityT1_;
    double fiberLength_;            //calcolato nella classe
    double fiberLengthT1_;          //valore della fiberLength al passo precedente
    double fiberVelocity_;
    double muscleTendonForce_;
    double activation_;             // da passare in qualche modo alla classe, potrebbe servire anche l'attivazione passata
    double activationT1_;
    double timescale_;          //da sistemare il valore hardcoded
    double lmtTime_;                   // da passare alla classe, forse si puo' evitare di utilizzare una variabile membro
    double lmtTimeT1_;
    unsigned nLmt_;
    
    std::string id_;
};


#endif

