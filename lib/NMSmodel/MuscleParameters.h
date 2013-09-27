#ifndef MuscleParameters_h
#define MuscleParameters_h

class MuscleParameters {
public:
    MuscleParameters();
    ~MuscleParameters();
    bool operator==(const MuscleParameters &other) const;
    double getC1()  const                    { return c1_;                  }
    double getC2()  const                    { return c2_;                  }
    double getShapeFactor() const            { return shapeFactor_;         }
    double getOptimalFiberLength() const     { return optimalFiberLength_;  }
    double getPennationAngle() const         { return pennationAngle_;      }
    double getTendonSlackLenght() const      { return tendonSlackLength_;   }
    double getMaxIsometricForce() const      { return maxIsometricForce_;   }
    double getStrengthCoefficient() const    { return strengthCoefficient_; }
    double getEmDelay() const                { return emDelay_;             }
    
    void setC1(double c1)                                   { c1_                  = c1;                  }
    void setC2(double c2)                                   { c2_                  = c2;                  }
    void setShapeFactor(double shapeFactor)                 { shapeFactor_         = shapeFactor;         }
    void setOptimalFiberLength(double optimalFiberLength)   { optimalFiberLength_  = optimalFiberLength;  }
    void setPennationAngle(double pennationAngle)           { pennationAngle_      = pennationAngle;      }
    void setTendonSlackLength(double tendonSlackLength)     { tendonSlackLength_   = tendonSlackLength;   }
    void setMaxIsometricForce(double maxIsometricForce)     { maxIsometricForce_   = maxIsometricForce;   }
    void setStrengthCoefficient(double strengthCoefficient) { strengthCoefficient_ = strengthCoefficient; }
    void setEmDelay(double emDelay)                         { emDelay_             = emDelay;             }    
    
private:
    double c1_;
    double c2_;
    double shapeFactor_;
    double optimalFiberLength_;
    double pennationAngle_;   
    double tendonSlackLength_;
    double maxIsometricForce_;
    double strengthCoefficient_;
    double emDelay_;
};

#endif
