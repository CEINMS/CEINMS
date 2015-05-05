//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani, Massimo Sartori - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//         massimo.sartori@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_StiffTendon_h
#define ceinms_StiffTendon_h
#include "Curve.h"
#include <string>

namespace ceinms {
    class StiffTendon {

    public:
        typedef Curve<CurveMode::Offline> CurveOffline;
        StiffTendon();
        StiffTendon(std::string id);
        StiffTendon(double optimalFibreLength,
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
        virtual ~StiffTendon();


        void setParametersToComputeForces(double optimalFibreLength,
            double pennationAngle,
            double tendonSlackLength,
            double percentageChange,
            double damping,
            double maxIsometricForce,
            double strengthCoefficient,
            double maxContractionVelocity);

        void setTime(const double& time) {} // it is useless in the stiff tendon, but I need to respect the interface
        void setMuscleTendonLength(double muscleTendonLength);
        void setActivation(double activation);
        void updateFibreLength();
        double getPenalty() const { return .0; }

        double getFibreLength() { return fibreLength_; }
        void setStrengthCoefficient(double strengthCoefficient) { /*intentionally left blank*/ };
        void setTendonSlackLength(double tendonSlackLength) { tendonSlackLength_ = tendonSlackLength; }
        void setOptimalFibreLength(double optimalFibreLength) { optimalFibreLength_ = optimalFibreLength; }
        void setPennationAngle(double pennationAngle) { pennationAngle_ = pennationAngle; }
        void setMaxIsometricForce(double maxIsometricForce) {/*intentionally left blank*/ };
        void setCurves(const CurveOffline& activeForceLengthCurve,
            const CurveOffline& passiveForceLengthCurve,
            const CurveOffline& forceVelocityCurve,
            const CurveOffline& tendonForceStrainCurve) { };
        void setMaxContractionVelocity(double maxContractionVelocity) {/*empty*/ }
        void pushState() {};
        void resetState();

        void setTolerance(double tolerance = 0.0){};

    private:
        double optimalFibreLength_;
        double pennationAngle_;
        double tendonSlackLength_;

        double fibreLength_;
        double muscleTendonLength_;

        std::string id_;

    };
}

#endif