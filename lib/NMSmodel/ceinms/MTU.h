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
 * Author(s): C. Pizzolato, M. Reggiani                                       *
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

#ifndef ceinms_MTU_h
#define ceinms_MTU_h

#include <string>
#include <iostream>

#include "ceinms/Curve.h"

namespace ceinms {
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
            double maxContractionVelocity = 10.0);

        void setParametersToComputeActivation(double c1,
            double c2,
            double shapeFactor);

        std::string getMuscleName() const { return id_; }
        bool   compareMusclesId(const std::string& id) const { return id_ == id; }
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
        double getActivation() const { return activation_; }
        void   setEmDelay(double emDelay) { emDelay_ = emDelay; }
        double getEmDelay() const { return emDelay_; }
        void   setShapeFactor(double shapeFactor);
        double getShapeFactor() const { return shapeFactor_; }
        void   setC1(double c1);
        double getC1() const { return c1_; }
        void   setC2(double c2);
        double getC2() const { return c2_; }
        void   setStrengthCoefficient(double strengthCoefficient);
        double getStrengthCoefficient() const { return strengthCoefficient_; }
        void   setTendonSlackLength(double tendonSlackLength);
        double getTendonSlackLength() const { return tendonSlackLength_; };
        void   setOptimalFibreLength(double optimalFibreLength);
        double getOptimalFibreLength() const { return optimalFibreLength_; }
        void   setPennationAngle(double pennationAngle);
        double getPennationAngle() const { return pennationAngle_; }
        double getMaxIsometricForce() const { return maxIsometricForce_; }
        void   setMaxIsometricForce(double maxIsometricForce);
        double getMaxContractionVelocity() const { return maxContractionVelocity_; }
        void   setMaxContractionVelocity(double maxContractionVelocity);

        double getFiberLength() const { return fibreLength_; }
        double getNormFiberLength() const { return fibreLength_ / optimalFibreLength_; }
        double getFiberVelocity() const { return fibreVelocity_; }
        double getNormFiberVelocity() const { return normFibreVelocity_; }
        double getPennationAngleAtT() const { return pennationAngleAtT_; };
        double getMuscleForce() const { return muscleForce_; }
        double getPercentageChange() const { return percentageChange_; }
        double getDamping() const { return damping_; }
        void   setDamping(double damping);


        void setTendonTolerance(double tolerance = 0.0);

        void   setMuscleForce(double muscleForce) { muscleForce_ = muscleForce; }
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
        double pennationAngleAtT_;
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
}

#include "MTU.cpp"
#endif
