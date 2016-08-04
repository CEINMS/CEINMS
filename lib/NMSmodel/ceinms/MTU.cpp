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

#include "ceinms/Curve.h"
#include <iostream>
using std::cout;
using std::endl;
#include <stdlib.h>
#include <cmath>
using std::cos;
#include "MTUutils.h"

namespace ceinms {
    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    MTU<Activation, Tendon, mode>::MTU()
        :id_(""), emDelay_(.0), c1_(0.), c2_(0.), shapeFactor_(0.), activation_(0.),
        optimalFibreLength_(0.), pennationAngle_(0.), tendonSlackLength_(0.),
        percentageChange_(0.), fibreLength_(0.), pennationAngleAtT_(0.), muscleTendonLength_(0.),
        fibreVelocity_(0.), damping_(0.), maxIsometricForce_(0.), time_(.0), timeScale_(0.),
        strengthCoefficient_(0.), muscleForce_(0.), maxContractionVelocity_(0.)  { }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    MTU<Activation, Tendon, mode>::MTU(std::string id)
        :id_(id), emDelay_(.0), c1_(0.), c2_(0.), shapeFactor_(0.), activation_(0.),
        optimalFibreLength_(0.), pennationAngle_(0.), tendonSlackLength_(0.),
        percentageChange_(0.), fibreLength_(0.), pennationAngleAtT_(0.), muscleTendonLength_(0.),
        fibreVelocity_(0.), damping_(0.), maxIsometricForce_(0.), time_(.0), timeScale_(0.),
        strengthCoefficient_(0.), muscleForce_(0.), maxContractionVelocity_(0.), tendonDynamic_(id)  { }



    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    MTU<Activation, Tendon, mode>::MTU(const MTU<Activation, Tendon, mode>& orig) {

        id_ = orig.id_;
        emDelay_ = orig.emDelay_;
        activationDynamic_ = orig.activationDynamic_;
        activation_ = orig.activation_;
        c1_ = orig.c1_;
        c2_ = orig.c2_;
        shapeFactor_ = orig.shapeFactor_;

        tendonDynamic_ = orig.tendonDynamic_;
        fibreVelocity_ = orig.fibreVelocity_;
        normFibreVelocity_ = orig.normFibreVelocity_;
        fibreLength_ = orig.fibreLength_;
        fibreLengthTrace_ = orig.fibreLengthTrace_;
        muscleTendonLength_ = orig.muscleTendonLength_;
        muscleForce_ = orig.muscleForce_;
        pennationAngleAtT_ = orig.pennationAngleAtT_;

        optimalFibreLength_ = orig.optimalFibreLength_;
        pennationAngle_ = orig.pennationAngle_;
        tendonSlackLength_ = orig.tendonSlackLength_;
        percentageChange_ = orig.percentageChange_;
        damping_ = orig.damping_;
        maxIsometricForce_ = orig.maxIsometricForce_;
        strengthCoefficient_ = orig.strengthCoefficient_;
        maxContractionVelocity_ = orig.maxContractionVelocity_;
        forceVelocityCurve_ = orig.forceVelocityCurve_;
        activeForceLengthCurve_ = orig.activeForceLengthCurve_;
        passiveForceLengthCurve_ = orig.passiveForceLengthCurve_;
        tendonForceStrainCurve_ = orig.tendonForceStrainCurve_;

        timeScale_ = orig.timeScale_;
        time_ = orig.time_;
    }



    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    MTU<Activation, Tendon, mode>& MTU<Activation, Tendon, mode>::operator=(const MTU<Activation, Tendon, mode>& orig) {

        id_ = orig.id_;
        emDelay_ = orig.emDelay_;
        activationDynamic_ = orig.activationDynamic_;
        activation_ = orig.activation_;
        c1_ = orig.c1_;
        c2_ = orig.c2_;
        shapeFactor_ = orig.shapeFactor_;

        tendonDynamic_ = orig.tendonDynamic_;
        fibreVelocity_ = orig.fibreVelocity_;
        normFibreVelocity_ = orig.normFibreVelocity_;
        fibreLength_ = orig.fibreLength_;
        fibreLengthTrace_ = orig.fibreLengthTrace_;
        muscleTendonLength_ = orig.muscleTendonLength_;
        muscleForce_ = orig.muscleForce_;
        pennationAngleAtT_ = orig.pennationAngleAtT_;

        optimalFibreLength_ = orig.optimalFibreLength_;
        pennationAngle_ = orig.pennationAngle_;
        tendonSlackLength_ = orig.tendonSlackLength_;
        percentageChange_ = orig.percentageChange_;
        damping_ = orig.damping_;
        maxIsometricForce_ = orig.maxIsometricForce_;
        strengthCoefficient_ = orig.strengthCoefficient_;
        maxContractionVelocity_ = orig.maxContractionVelocity_;
        forceVelocityCurve_ = orig.forceVelocityCurve_;
        activeForceLengthCurve_ = orig.activeForceLengthCurve_;
        passiveForceLengthCurve_ = orig.passiveForceLengthCurve_;
        tendonForceStrainCurve_ = orig.tendonForceStrainCurve_;

        timeScale_ = orig.timeScale_;
        time_ = orig.time_;
        return *this;
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setParametersToComputeForces(double optimalFibreLength,
        double pennationAngle,
        double tendonSlackLength,
        double percentageChange,
        double damping,
        double maxIsometricForce,
        double strengthCoefficient,
        double maxContractionVelocity) {
        optimalFibreLength_ = optimalFibreLength;
        pennationAngle_ = pennationAngle;
        tendonSlackLength_ = tendonSlackLength;
        percentageChange_ = percentageChange;
        damping_ = damping;
        maxIsometricForce_ = maxIsometricForce;
        strengthCoefficient_ = strengthCoefficient;
        maxContractionVelocity_ = maxContractionVelocity;

        tendonDynamic_.setParametersToComputeForces(optimalFibreLength,
            pennationAngle,
            tendonSlackLength,
            percentageChange,
            damping,
            maxIsometricForce,
            strengthCoefficient,
            maxContractionVelocity);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setParametersToComputeActivation(double c1, double c2, double shapeFactor) {

        c1_ = c1;
        c2_ = c2;
        shapeFactor_ = shapeFactor;
        activationDynamic_.setFilterParameters(c1_, c2_);
        activationDynamic_.setShapeFactor(shapeFactor_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::resetState() {

        activationDynamic_.resetState();
        tendonDynamic_.resetState();
        fibreLengthTrace_.reset();
        activation_ = .0;
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setTime(const double& time) {

        timeScale_ = time - time_;
        time_ = time;
        tendonDynamic_.setTime(time);
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setEmg(double emg) {

        if (!activationDynamic_.setEmg(emg))
            std::cout << "WARNING! Wrong excitation data provided for muscle " << getMuscleName() << ", it should be between 0 and 1" << std::endl;
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setMuscleTendonLength(double muscleTendonLength) {

        muscleTendonLength_ = muscleTendonLength;
        tendonDynamic_.setMuscleTendonLength(muscleTendonLength);
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateActivation() {

        activationDynamic_.updateActivation();
        activation_ = activationDynamic_.getActivation();
        tendonDynamic_.setActivation(activation_);
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLengthAndVelocity() {

        updateFibreLength();
        updateFibreVelocity();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLength() {

        tendonDynamic_.updateFibreLength();
        fibreLength_ = tendonDynamic_.getFibreLength();
        fibreLengthTrace_.addPointNoUpdate(time_, fibreLength_);
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreVelocity() {

        fibreLengthTrace_.refresh();
        fibreVelocity_ = fibreLengthTrace_.getFirstDerivative(time_);
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLength_OFFLINEPREP() {

        updateFibreLength();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLengthAndVelocity_OFFLINE() {

        updateFibreLength_OFFLINE();
        updateFibreVelocity_OFFLINE();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLength_OFFLINE() {

        tendonDynamic_.updateFibreLength();
        fibreLength_ = tendonDynamic_.getFibreLength();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreVelocity_OFFLINE() {

        fibreVelocity_ = fibreLengthTrace_.getFirstDerivative(time_);
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLengthAndVelocity_HYBRID() {

        updateFibreLength_OFFLINE();
        fibreLengthTrace_.removeLastPointNoUpdate();
        fibreLengthTrace_.addPointNoUpdate(time_, fibreLength_);
        fibreLengthTrace_.refresh();
        updateFibreVelocity_OFFLINE();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLengthsAndVelocities_END_OF_HYBRID_MINIMIZATION() {

        fibreLengthTrace_.removeLastPointNoUpdate();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateMuscleForce() {

        double optimalFiberLengthAtT = optimalFibreLength_*(percentageChange_*(1.0 - activation_) + 1);
        ////:TODO: strong review with the code... lot of check for closeness to 0
        double normFiberLengthAtT = fibreLength_ / optimalFiberLengthAtT;
        double normFiberLength = fibreLength_ / optimalFibreLength_;
        //:TODO: THIS IS WRONG! timeScale_?  0.1 should be timeScale_
        // double normFiberVelocity = timescale_ *fiberVelocity_ / optimalFiberLengthAtT;
        double fiberVel = fibreVelocity_ / optimalFibreLength_;
        if (fiberVel > maxContractionVelocity_)
            fiberVel = maxContractionVelocity_;
        if (fiberVel < -maxContractionVelocity_)
            fiberVel = -maxContractionVelocity_;
        double normFiberVelocity = fiberVel / maxContractionVelocity_;

        double fv = forceVelocityCurve_.getValue(normFiberVelocity);
        double fp = passiveForceLengthCurve_.getValue(normFiberLength);
        double fa = activeForceLengthCurve_.getValue(normFiberLengthAtT);
        double pennationAngleAtT = computePennationAngle(optimalFibreLength_); //this is correct
        //    double pennationAngleAtT = computePennationAngle(optimalFiberLengthAtT); //this is the one we used since the new version...

        normFibreVelocity_ = normFiberVelocity;
        pennationAngleAtT_ = pennationAngleAtT; //for logging
        muscleForce_ = maxIsometricForce_*strengthCoefficient_*
            (fa*fv*activation_ + fp + damping_*normFiberVelocity)*
            cos(pennationAngleAtT);
    }

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateMtuStiffness() {

        double optimalFiberLengthAtT = optimalFibreLength_*(percentageChange_*(1.0 - activation_) + 1);
        ////:TODO: strong review with the code... lot of check for closeness to 0
        double normFiberLength = fibreLength_ / optimalFiberLengthAtT;
        //:TODO: THIS IS WRONG! timeScale_?  0.1 should be timeScale_
        // double normFiberVelocity = timescale_ *fiberVelocity_ / optimalFiberLengthAtT;
        double normFiberVelocity = 0.02*fibreVelocity_*0.1 / optimalFiberLengthAtT;

        double fv = forceVelocityCurve_.getValue(normFiberVelocity);
        double dfp = passiveForceLengthCurve_.getFirstDerivative(normFiberLength) / optimalFiberLengthAtT; // derivative respect to length, not normlength
        double dfa = activeForceLengthCurve_.getFirstDerivative(normFiberLength) / optimalFiberLengthAtT;

        double tendonLength_ = muscleTendonLength_ - fibreLength_;
        double tendonStrain_ = (tendonLength_ - tendonSlackLength_) / tendonSlackLength_;

        if (tendonStrain_ < 0)
        {
            //cout << "MTU: TendonStrain_ is " << tendonStrain_ << ". Set to 0." << endl;
            tendonStrain_ = 0;
        }

        double tendonStiffness_ = maxIsometricForce_*strengthCoefficient_*
            tendonForceStrainCurve_.getFirstDerivative(tendonStrain_) / tendonSlackLength_; // derivative respect to length, not normlength

        if (tendonStiffness_ < 0)
        {
            cout << "Warning: tendonStiffness_ is < 0" << endl;
        }

        double pennationAngleAtT = computePennationAngle(optimalFibreLength_);

        normFibreVelocity_ = normFiberVelocity;

        // Approximation: d(cos(pennAng)) negligible
        double muscleStiffness_ = maxIsometricForce_*strengthCoefficient_*
               (dfa*fv*activation_ + dfp)*
               cos(radians(pennationAngleAtT));

        mtuStiffness_ =  (muscleStiffness_ * tendonStiffness_) / (muscleStiffness_ + tendonStiffness_);
    }

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::pushState() {

        activationDynamic_.pushState();
        tendonDynamic_.pushState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::resetFibreLengthTrace() {

        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::updateFibreLengthTrace() {

        fibreLengthTrace_.refresh();
        activationDynamic_.resetState();
        tendonDynamic_.resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setCurves(const CurveOffline& activeForceLengthCurve,
        const CurveOffline& passiveForceLengthCurve,
        const CurveOffline& forceVelocityCurve,
        const CurveOffline& tendonForceStrainCurve) {

        activeForceLengthCurve_ = activeForceLengthCurve;
        passiveForceLengthCurve_ = passiveForceLengthCurve;
        forceVelocityCurve_ = forceVelocityCurve;
        tendonForceStrainCurve_ = tendonForceStrainCurve;

        tendonDynamic_.setCurves(activeForceLengthCurve, passiveForceLengthCurve, forceVelocityCurve, tendonForceStrainCurve);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setActivation(double activation) {

        activation_ = activation;
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setShapeFactor(double shapeFactor) {

        shapeFactor_ = shapeFactor;
        activationDynamic_.setShapeFactor(shapeFactor_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setC1(double c1) {

        c1_ = c1;
        activationDynamic_.setFilterParameters(c1_, c2_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setC2(double c2) {

        c2_ = c2;
        activationDynamic_.setFilterParameters(c1_, c2_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setStrengthCoefficient(double strengthCoefficient) {

        strengthCoefficient_ = strengthCoefficient;
        tendonDynamic_.setStrengthCoefficient(strengthCoefficient_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setTendonSlackLength(double tendonSlackLength) {

        tendonSlackLength_ = tendonSlackLength;
        tendonDynamic_.setTendonSlackLength(tendonSlackLength_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setOptimalFibreLength(double optimalFiberLength) {

        optimalFibreLength_ = optimalFiberLength;
        tendonDynamic_.setOptimalFibreLength(optimalFibreLength_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setPennationAngle(double pennationAngle) {

        pennationAngle_ = pennationAngle;
        tendonDynamic_.setPennationAngle(pennationAngle_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setMaxIsometricForce(double maxIsometricForce) {

        maxIsometricForce_ = maxIsometricForce;
        tendonDynamic_.setMaxIsometricForce(maxIsometricForce_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setMaxContractionVelocity(double maxContractionVelocity) {

        maxContractionVelocity_ = maxContractionVelocity;
        tendonDynamic_.setMaxContractionVelocity(maxContractionVelocity_);
        resetState();
    }

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setDamping(double damping) {
        damping_ = damping;
        tendonDynamic_.setDamping(damping_);
        resetState();
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    double MTU<Activation, Tendon, mode>::getPenalty() const {

        double penalty(tendonDynamic_.getPenalty());
        double const normalisedFibreRatio(fabs(fibreLength_ / optimalFibreLength_ - 1.0));
        if (normalisedFibreRatio > 0.5)
            penalty += normalisedFibreRatio*normalisedFibreRatio * 100;
        return penalty;
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    inline double MTU<Activation, Tendon, mode>::computePennationAngle(double optimalFiberLength) {

        return PennationAngle::compute(fibreLength_, optimalFiberLength, pennationAngle_);

    }

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void MTU<Activation, Tendon, mode>::setTendonTolerance(double tolerance) {

        tendonDynamic_.setTolerance(tolerance);
    }

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    std::ostream& operator<< (std::ostream& output, const MTU<Activation, Tendon, mode>& m)
    {
        output << "Name: " << m.id_ << endl;
        output << "emDelay: " << m.emDelay_ << endl;
        output << "C1: " << m.c1_ << " C2: " << m.c2_ << endl;
        output << "Shape Factor: " << m.shapeFactor_ << endl;
        output << "activeForceLength" << endl << m.activeForceLengthCurve_ << endl;
        output << "passiveForceLength" << endl << m.passiveForceLengthCurve_ << endl;
        output << "forceVelocity" << endl << m.forceVelocityCurve_ << endl;
        output << "tendonForceStrain " << endl << m.tendonForceStrainCurve_ << endl;
        output << "optimalFibreLength: " << m.optimalFibreLength_ << endl;
        output << "pennationAngle: " << m.pennationAngle_ << endl;
        output << "tendonSlackLength: " << m.tendonSlackLength_ << endl;
        output << "percentageChange: " << m.percentageChange_ << endl;
        output << "damping: " << m.damping_ << endl;
        output << "maxIsometricForce: " << m.maxIsometricForce_ << endl;
        output << "strengthCoefficient: " << m.strengthCoefficient_ << endl;
        // :TODO: valli a mettere anche nel costruttore di copia

        return output;
    }

}
