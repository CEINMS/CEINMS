/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): C. Pizzolato, M. Reggiani, M. Sartori                           *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

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
