//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_ElasticTendon_BiSec_h
#define ceinms_ElasticTendon_BiSec_h
#include <string>
#include "Curve.h"

namespace ceinms {
    //template <typename CurveMode::Mode mode>
    class ElasticTendon_BiSec {

    public:
        typedef Curve<CurveMode::Offline> CurveOffline;
        ElasticTendon_BiSec();
        ElasticTendon_BiSec(std::string id);
        ElasticTendon_BiSec(double optimalFibreLength,
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
            double strengthCoefficient,
            double maxContractionVelocity);

        void setTime(const double& time) { time_ = time; }
        void setMuscleTendonLength(double muscleTendonLength);
        void setActivation(double activation);
        void updateFibreLength();
        double getPenalty() const { return tendonPenalty_; }

        double getFibreLength() { return fibreLength_; }
        void setStrengthCoefficient(double strengthCoefficient);
        void setTendonSlackLength(double tendonSlackLength);
        void setOptimalFibreLength(double optimalFibreLength);
        void setPennationAngle(double pennationAngle);
        void setMaxIsometricForce(double maxIsometricForce);
        void setMaxContractionVelocity(double maxContractionVelocity);
        void setCurves(const CurveOffline& activeForceLengthCurve,
            const CurveOffline& passiveForceLengthCurve,
            const CurveOffline& forceVelocityCurve,
            const CurveOffline& tendonForceStrainCurve);
        void setTolerance(double tolerance);
        double getTolerance() const;
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
        double maxContractionVelocity_;
        CurveOffline activeForceLengthCurve_;
        CurveOffline passiveForceLengthCurve_;
        CurveOffline forceVelocityCurve_;
        CurveOffline tendonForceStrainCurve_;

        Curve<CurveMode::Online, CurveMode::Cubic, 3>  fibreLengthTrace_;
        double fibreLength_;
        double muscleTendonLength_;
        double activation_;
        double tendonPenalty_;
        double time_;
        double tolerance_;
        std::string id_;

    };
}

#endif