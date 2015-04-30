#ifndef MTU_h
#define MTU_h

#include <string>
#include <iostream>

#include "Curve.h"

template<typename Activation, typename Tendon, CurveMode::Mode mode>
class MTU;

template<typename Activation, typename Tendon, CurveMode::Mode mode>
std::ostream& operator<<  (std::ostream& output, const MTU<Activation, Tendon, mode>& m);



template<typename Activation, typename Tendon, CurveMode::Mode mode>
class MTU {
    
    typedef Curve<mode> CurveT;
    typedef Curve<CurveMode::Offline> CurveOffline;
    
public:
    MTU();
    MTU(std::string id);
    MTU(const MTU& orig);
    MTU& operator=(const MTU& orig);
    virtual ~MTU() {};
    
    void setParametersToComputeForces(double optimalFiberLength,
                                      double pennationAngle,
                                      double tendonSlackLength,
                                      double percentageChange,
                                      double damping, 
                                      double maxIsometricForce, 
                                      double strengthCoefficient,
                                      double maxContractionVelocity=10.0);
                                   
    void setParametersToComputeActivation(double c1,
                                          double c2,
                                          double shapeFactor);
   
    std::string getMuscleName() const { return id_; }
    bool   compareMusclesId(const std::string& id) const { return id_ == id;}
    void   printName(std::ostream& output) { output << id_; }

    void   setTime(const double& time);
    void   setEmg(double emg);
    void   setMuscleTendonLength(double muscleTendonLength); 
    void   updateActivation();
    void   updateFibreLengthAndVelocity();
    void   updateFibreLengthAndVelocity_OFFLINE();   
    void   updateFibreLengthAndVelocity_HYBRID();
    void   updateFibreLength_OFFLINEPREP();
    void   updateMuscleForce();
    void   pushState();
    
    
 //   void   setEmgValueNoUpdate(double emg);
 //   void   setEmgValueNoNeuralFilterUpdate(double emg);
    double getEmg() const { return activationDynamic_.getEmg(); }
    double getPastEmg() const { return activationDynamic_.getPastEmg(); }
    double getNeuralActivation() const { return activationDynamic_.getNeuralActivation(); }
    void   setCurves(const CurveOffline& activeForceLengthCurve, const CurveOffline& passiveForceLengthCurve, const CurveOffline& forceVelocityCurve, const CurveOffline& tendonForceLengthCurve);
    void   setActivation(double activation); //recompute all the data that depend on activation
    double getActivation() const {return activation_;}
    void   setEmDelay(double emDelay) { emDelay_ = emDelay;}
    double getEmDelay() const { return emDelay_;}
    void   setShapeFactor(double shapeFactor);
    double getShapeFactor() const {return shapeFactor_;}
    void   setC1(double c1);
    double getC1() const {return c1_;}
    void   setC2(double c2);
    double getC2() const {return c2_;}
    void   setStrengthCoefficient(double strengthCoefficient);
    double getStrengthCoefficient() const {return strengthCoefficient_;}
    void   setTendonSlackLength(double tendonSlackLength);
    double getTendonSlackLength() const {return tendonSlackLength_;};
    void   setOptimalFibreLength(double optimalFibreLength);
    double getOptimalFibreLength() const { return optimalFibreLength_;}
    void   setPennationAngle(double pennationAngle);
    double getPennationAngle() const {return pennationAngle_; }
    double getMaxIsometricForce() const { return maxIsometricForce_; }
    void   setMaxIsometricForce(double maxIsometricForce);

    double getFiberLength() const {return fibreLength_;}
    double getFiberVelocity() const {return fibreVelocity_; }
    double getNormFiberVelocity() const {return normFibreVelocity_; } 
    double getMuscleForce() const {return muscleForce_;}
    double getPercentageChange() const { return percentageChange_;}
    double getDamping() const {return damping_;}
    
    

  
    void   setMuscleForce(double muscleForce) {muscleForce_ = muscleForce;}
    void   updateFibreLengthTrace();
    void   resetFibreLengthTrace();
    double getPenalty() const;
    inline double computePennationAngle(double optimalFiberLengthAtT);
    
    friend std::ostream& operator<< <> (std::ostream& output, const MTU& m);
    
private:
    void updateFibreLength();
    void updateFibreLength_OFFLINE();
    void updateFibreVelocity();
    void updateFibreVelocity_OFFLINE();
    void updateLastFibreVelocityValue(double time);
    void resetState();
    
    std::string id_;
    double emDelay_;
    
    // MUSCLE ACTIVATION: parameters and utility methods 
    Activation activationDynamic_;
   
    double activation_;    /**< \f$a(t)\f$ activation of the muscle */

    // Parameters to compute muscle activation
    double c1_;            /**< \f$C_1\f$ second-order filter coefficient */ 
    double c2_;            /**< \f$C_2\f$ second-order filter coefficient */
    double shapeFactor_;   /**< \f$A\f$ non-linear shape factor */
    
    // FIBER LENGTH, FIBER VELOCITY, and TENDON FORCE: 
    Tendon tendonDynamic_;
    double fibreVelocity_;       /**< \f$v^m\f$ */
    double normFibreVelocity_; 
    double fibreLength_;         /**< \f$l^m\f$ */    
    CurveT  fibreLengthTrace_;
    double muscleForce_;    /**< \f$F^{mt}\f$ */
    
    // parameters
    double optimalFibreLength_;  /**< \f$l_0^m\f$ */
    double pennationAngle_;      /**< \f$\phi\f$ */
    double tendonSlackLength_;   /**< \f$l^t\f$ */
    double percentageChange_;    /**< \f$\gamma\f$ */
    double damping_;             /**< \f$d^m\f$ */
    double maxIsometricForce_;   /**< \f$F_0^m\f$ */
    double strengthCoefficient_;  /**< \f$\delta\f$ */
    double maxContractionVelocity_;
    CurveOffline  forceVelocityCurve_;
    CurveOffline  activeForceLengthCurve_;
    CurveOffline  passiveForceLengthCurve_;
    CurveOffline  tendonForceStrainCurve_;
    
    double timeScale_;
    double time_;
};

#include "MTU.cpp"

#endif