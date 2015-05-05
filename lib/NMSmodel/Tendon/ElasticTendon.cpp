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
 * Author(s):                                                                 *
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
 
#include "ElasticTendon.h"
#include "StiffTendon.h"
#include "Odeint.h"
#include "StepperDopr5.h"
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;

#include <cmath>
using std::sin;
using std::cos;
#include <vector>
//#include <boost/config/no_tr1/complex.hpp>
using std::vector;
#include "float.h"

#include "Curve.h"
#include "../MTUutils.h"
#define DEBUG


namespace ceinms {
    LDFM::LDFM(double optimalFiberLength,
        double pennationAngle,
        double tendonSlackLength,
        double percentageChange,
        double damping,
        double maxIsometricForce,
        double strengthCoefficient,
        const CurveOffline& activeForceLengthCurve,
        const CurveOffline& passiveForceLengthCurve,
        const CurveOffline& forceVelocityCurve,
        const CurveOffline& tendonForceStrainCurve) :

        optimalFiberLength_(optimalFiberLength),
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
        activation_(.0),
        muscleTendonLength_(.0),
        muscleTendonVelocity_(.0),
        muscleTendonForce_(.0),
        fiberLength_(.0),
        fiberVelocity_(.0),
        fiberStiffness_(.0),
        tendonLength_(.0),
        tendonVelocity_(.0),
        tendonStiffness_(.0),
        optimalFiberLengthAtT_(.0)
    { }


    double LDFM::computePennationAngle() const {

        return PennationAngle::compute(fiberLength_, optimalFiberLengthAtT_, pennationAngle_);
    }


    void LDFM::setInitConditions(double muscleTendonLength, double muscleTendonVelocity, double activation) {

        activation_ = activation;
        muscleTendonLength_ = muscleTendonLength;
        muscleTendonVelocity_ = muscleTendonVelocity;
        //   muscleTendonVelocity_ = .0;
        fiberVelocity_ = muscleTendonVelocity; //e' nulla se si usa il modello online, poiche' mancano punti per la spline
        tendonVelocity_ = 0.0;

        /* Make first guess of muscle and tendon lengths. Make muscle length
        * lmo so that the LDFEM iterative method just moves down one side of
        * the force length curve (i.e. the part of the curve you use is monotonic).
        * And if you are including passive muscle force, you start in the middle
        * of the combined force-length curve.
        */
        double muscleThick = optimalFiberLength_  * sin(pennationAngle_);
        optimalFiberLengthAtT_ = optimalFiberLength_ * (percentageChange_ * (1.0 - activation) + 1);

        if (muscleTendonLength_ < tendonSlackLength_) {
            fiberLength_ = muscleThick;
            tendonLength_ = muscleTendonLength_;
        }
        else {
            fiberLength_ = optimalFiberLength_;
            double cf = cos(computePennationAngle());
            tendonLength_ = muscleTendonLength_ - fiberLength_*cf;

            /* Check to make sure tendon is not shorter than its slack length. If it is,
            * set the length to its slack length and re-compute muscle length.
            */

            if (tendonLength_ < tendonSlackLength_) {

                tendonLength_ = tendonSlackLength_;
                double cf = cos(atan(muscleThick / muscleTendonLength - tendonLength_));
                fiberLength_ = (muscleTendonLength_ - tendonLength_) / cf;
                if (fiberLength_ < muscleThick)
                    fiberLength_ = muscleThick;
            }
        }

    }


    double LDFM::estimateFiberLength() {

        double errorForce = DBL_MAX;
        double errorForceOld = DBL_MAX;
        bool runCondition = true;
        const double relTolerance = 1e-5;
        const unsigned maxCount = 1000;
        unsigned count = 0;
        double fiberLengthTmp = .0, fiberLengthOld = .0;

        do {

            /* force = maxisoforce * (flcurve+pflcurve) * velocity * pennation
            * calculate active force (af) then passive force (pf) */

            computeMuscleTendonForce();
            double tendonStrain = (tendonLength_ / tendonSlackLength_) - 1.0;
            double tendonForce = tendonStrain <= 0.0 ? 0.0 : strengthCoefficient_*maxIsometricForce_*tendonForceStrainCurve_.getValue(tendonStrain);

            errorForce = tendonForce - muscleTendonForce_;

            if ((errorForce * errorForceOld) < 0.0) {  // if the signs are different
                double percent = fabs(errorForce) / (fabs(errorForce) + fabs(errorForceOld));
                fiberLengthTmp = fiberLength_;
                fiberLength_ += percent * (fiberLengthOld - fiberLength_);
                fiberLengthOld = fiberLengthTmp;
            }
            else { /* estimate stiffness of the tendon and muscle */

                tendonStiffness_ = strengthCoefficient_*maxIsometricForce_ / tendonSlackLength_ * tendonForceStrainCurve_.getFirstDerivative(tendonStrain);

                /* If tendon stiffness is too low, then the next length guess will
                * overshoot the equilibrium point. So we artificially raise it
                * using the normalized muscle force. (af+pf) is the normalized
                * force for the current muscle length, and we assume that the
                * equilibrium length is close to this current length. So we want
                * to get force = (af+pf) from the tendon as well. We hope this
                * will happen by setting the tendon stiffness to (af+pf) times
                * its maximum stiffness. TODO: replace the numbers 1200 and 32.
                */

                //non l' capita del tutto eh.. anzi, per nulla
                //    double minTendonStiffness = (fa+fp)*1200.0*(maxIsometricForce_*strengthCoefficient_*(TENDON_STIFF_SCALE))/
                //                                (32.0*tendonSlackLength_);

                double normFiberLength = fiberLength_ / optimalFiberLengthAtT_;
                double fa = activeForceLengthCurve_.getValue(normFiberLength);
                double fp = passiveForceLengthCurve_.getValue(normFiberLength);
                double minTendonStiffness = (fa + fp)*1200.0*(maxIsometricForce_*strengthCoefficient_) /
                    (32.0*tendonSlackLength_);

                if (tendonStiffness_ < minTendonStiffness)
                    tendonStiffness_ = minTendonStiffness;

                // used activation in the calculation of fiber stiffness
                fiberStiffness_ = (maxIsometricForce_ * strengthCoefficient_*activation_) / optimalFiberLength_ *
                    (activeForceLengthCurve_.getFirstDerivative(normFiberLength) +
                    passiveForceLengthCurve_.getFirstDerivative(normFiberLength));

                /* determine how much the muscle and tendon lengths have to
                 * change to make the errorforce zero.   */

                double lengthChange = fabs(errorForce / (fiberStiffness_ / cos(computePennationAngle()) + tendonStiffness_));

                if (fabs(lengthChange / optimalFiberLength_) > 0.5)
                    lengthChange = 0.5 * optimalFiberLength_;

                /* now change the muscle length depending on the sign of the error
                * and the sign of the muscle stiffness (which side of the force
                * length curve you're on).
                */

                fiberLengthOld = fiberLength_;

                if (errorForce > 0.0)
                    fiberLength_ += lengthChange;
                else
                    fiberLength_ -= lengthChange;
            }

            //        optimalFiberLengthAtT_ = optimalFiberLength_ * (percentageChange_ * (1.0 - activation_) + 1 );
            //        double pennationAngleAtT = computePennationAngle();
            tendonLength_ = muscleTendonLength_ - fiberLength_ * cos(computePennationAngle());
            errorForceOld = errorForce;
            runCondition = fabs(errorForce) > (relTolerance*(maxIsometricForce_*strengthCoefficient_));

            /* Check to make sure tendon is not shorter than its slack length. If it is,
            * set the length to its slack length and re-compute muscle length.
            */

            if (tendonLength_ < tendonSlackLength_) {
                fiberLength_ = (muscleTendonLength_ - tendonLength_) / cos(computePennationAngle());
                tendonLength_ = tendonSlackLength_;
            }

            if (count >= maxCount) {
                muscleTendonForce_ = 0.0;
                //           cout << "LDFM: Could not find force for muscle: " <<  endl;
                runCondition = false;
            }

            /* If the error force is less than or equal to a certain percentage
            * of the muscle's max isometric force, then you're done.
            */
            updateInitConditions();
            ++count;

        } while (runCondition);
        return fiberLength_;
    }


    void LDFM::updateInitConditions() {

        double beta = 1;
        if ((fiberStiffness_ + tendonStiffness_) != 0)
            beta = fiberStiffness_ / (fiberStiffness_ + tendonStiffness_);
        tendonVelocity_ = muscleTendonVelocity_* beta;
        double cf = cos(computePennationAngle());
        fiberVelocity_ = (1.0 - beta) * cf * muscleTendonVelocity_;
    }


    void LDFM::computeMuscleTendonForce() {

        double normFiberLength = fiberLength_ / optimalFiberLengthAtT_;

        //:TODO: THIS IS WRONG! timeScale_?  0.1 should be timeScale_
        // double normFiberVelocity = timescale_ *fiberVelocity_ / optimalFiberLengthAtT;
        double normFiberVelocity = 0.02 * fiberVelocity_ * 0.1 / optimalFiberLengthAtT_;

        double fa = activeForceLengthCurve_.getValue(normFiberLength);
        double fp = passiveForceLengthCurve_.getValue(normFiberLength);
        double fv = forceVelocityCurve_.getValue(normFiberVelocity);

        double pennationAngleAtT = computePennationAngle();
        muscleTendonForce_ = maxIsometricForce_ * strengthCoefficient_ *
            (fa * fv * activation_ + fp + damping_ * normFiberVelocity) * cos(pennationAngleAtT);

    }


    double LDFM::oldEstimation(double muscleTendonLength, double muscleTendonVelocity, double activation) {

        const double relTolerance = 1e-3;
        int i, j, count;
        double fiber_stiffness, mthick, tmpfiber_length, min_tendon_stiffness;
        double af, pf, fv, cf, tendon_stiffness, percent, tendonstrain, oldfiber_length;
        double lengthchange, errorforce, olderrorforce, tendonforce, force, beta;

        muscleTendonLength_ = muscleTendonLength;
        muscleTendonVelocity_ = muscleTendonVelocity;
        activation_ = activation;

        if (muscleTendonLength_ <= tendonSlackLength_)
        {
            //        ds->force[i][0] = 0.0;
            fiberLength_ = 0.0;
            tendonLength_ = tendonSlackLength_ / optimalFiberLength_;
        }


        /* now calculate musculotendon velocity; initial guess of velocity distribution

    * puts all velocity in muscle */

        fiberVelocity_ = muscleTendonVelocity_;
        tendonVelocity_ = 0.0;


        /* Make first guess of muscle and tendon lengths. Make muscle length

        * lmo so that the LDFEM iterative method just moves down one side of

        * the force length curve (i.e. the part of the curve you use is monotonic).

        * And if you are including passive muscle force, you start in the middle

        * of the combined force-length curve.

        */



        mthick = optimalFiberLength_ * sin(pennationAngle_);

        if (muscleTendonLength_ < tendonSlackLength_)
        {
            fiberLength_ = mthick;
            tendonLength_ = muscleTendonLength_;
        }
        else
        {
            fiberLength_ = optimalFiberLength_;
            cf = cos(computePennationAngle());
            tendonLength_ = muscleTendonLength_ - fiberLength_*cf;


            /* Check to make sure tendon is not shorter than its slack length. If it is,

            * set the length to its slack length and re-compute muscle length.

            */


            if (tendonLength_ < tendonSlackLength_)
            {

                tendonLength_ = tendonSlackLength_;
                cf = cos(atan(mthick / muscleTendonLength_ - tendonLength_));
                fiberLength_ = (muscleTendonLength_ - tendonLength_) / cf;
                if (fiberLength_ < mthick)
                    fiberLength_ = mthick;
            }
        }

        /* the LDFEM (Loan Dynamic Force Estimation Method) loop is done twice. The
    * first time through, the tendon velocity is assumed to be zero, so all of
    * the velocity is `in' the muscle. The muscle force is then found iteratively
    * for this situation. This is done by guessing the length of the muscle and
    * of the tendon, and calculating their respective forces. If the forces
    * match (are within MAXIMUMERROR of each other), stop; else change the length
    * guesses based on the error and try again. Once the muscle force is know
    * a better estimate of its velocity can be made. The slope of the force-length
    * curve at this length is compared to the slope of the tendon force-leng
    * curve, and the velocities are guessed again.
    */

        mthick = optimalFiberLength_  * sin(pennationAngle_);


        for (j = 0; j < 2; j++)
        {
            count = 0;
            while (1)
            {

                count++;

                /* force = maxisoforce * (flcurve+pflcurve) * velocity * pennation
                * calculate active force (af) then passive force (pf) */

                af = activeForceLengthCurve_.getValue(fiberLength_)*activation_;
                if (af < 0.0)
                    af = 0.0;

                pf = passiveForceLengthCurve_.getValue(fiberLength_);
                if (pf < 0.0)
                    pf = 0.0;

                fv = forceVelocityCurve_.getValue(fiberVelocity_);

                force = (af*fv + pf) * (maxIsometricForce_*strengthCoefficient_) * cf;

                tendonstrain = (tendonLength_ / tendonSlackLength_ - 1.0);
                if (tendonstrain <= 0.0)
                    tendonforce = 0.0;
                else

                    tendonforce = maxIsometricForce_*tendonForceStrainCurve_.getValue(tendonstrain);

                errorforce = tendonforce - force;
                if (count == 1)
                    olderrorforce = errorforce;
                /* If the error force is less than or equal to a certain percentage
                * of the muscle's max isometric force, then you're done.
                */
                if (count > 1)
                {
                    if (fabs(errorforce) <= relTolerance*(maxIsometricForce_*strengthCoefficient_))
                        break;

                }

                if ((errorforce * olderrorforce) < 0)
                {
                    percent = fabs(errorforce) / (fabs(errorforce) + fabs(olderrorforce));
                    tmpfiber_length = oldfiber_length;
                    oldfiber_length = fiberLength_;
                    fiberLength_ += percent * (tmpfiber_length - fiberLength_);
                }
                else /* estimate stiffness of the tendon and muscle */
                {

                    tendon_stiffness = (maxIsometricForce_) / (tendonSlackLength_)*tendonForceStrainCurve_.getFirstDerivative(tendonstrain);

                    /* If tendon stiffness is too low, then the next length guess will
                    * overshoot the equilibrium point. So we artificially raise it
                    * using the normalized muscle force. (af+pf) is the normalized
                    * force for the current muscle length, and we assume that the
                    * equilibrium length is close to this current length. So we want
                    * to get force = (af+pf) from the tendon as well. We hope this
                    * will happen by setting the tendon stiffness to (af+pf) times
                    * its maximum stiffness. TODO: replace the numbers 1200 and 32.
                    */
                    double tendonStiffScale = 1;
                    min_tendon_stiffness = (af + pf)*1200.0*((maxIsometricForce_)*(strengthCoefficient_)*(tendonStiffScale)) /
                        (32.0*(tendonSlackLength_));

                    if (tendon_stiffness < min_tendon_stiffness)
                        tendon_stiffness = min_tendon_stiffness;

                    // used activation in the calculation of fiber stiffness

                    double normFiberLength = fiberLength_ / optimalFiberLength_;
                    fiber_stiffness = (maxIsometricForce_ * strengthCoefficient_*activation_) / optimalFiberLength_ *
                        (activeForceLengthCurve_.getFirstDerivative(normFiberLength) +
                        passiveForceLengthCurve_.getFirstDerivative(normFiberLength));
                    /* determine how much the muscle and tendon lengths have to
                         * change to make the errorforce zero.   */

                    lengthchange = fabs(errorforce / (fiber_stiffness / cf + tendon_stiffness));

                    if (fabs(lengthchange / optimalFiberLength_) > 0.5)
                        lengthchange = 0.5*optimalFiberLength_;

                    /* now change the muscle length depending on the sign of the error
                    * and the sign of the muscle stiffness (which side of the force
                    * length curve you're on).
                    */

                    oldfiber_length = fiberLength_;

                    if (errorforce > 0.0)
                        fiberLength_ += lengthchange;
                    else
                        fiberLength_ -= lengthchange;
                }

                cf = cos(computePennationAngle());
                tendonLength_ = muscleTendonLength_ - fiberLength_*cf;
                olderrorforce = errorforce;

                /* Check to make sure tendon is not shorter than its slack length. If it is,
                * set the length to its slack length and re-compute muscle length.
                */

                if (tendonLength_ < tendonSlackLength_)
                {

                    tendonLength_ = tendonSlackLength_;
                    cf = cos(atan(mthick / (muscleTendonLength_ - tendonLength_)));
                    fiberLength_ = (muscleTendonLength_ - tendonLength_) / cf;
                }
                if (count >= 100)
                {
                    force = 0.0;
                    break;
                }
            }
            /* if the first iteration, adjust the guess of muscle and tendon
                * velocity now that you know the (quasi) real lengths.
                * The total musculotendon velocity is distributed between the fibers
                * and tendon based on their relative stiffnesses (the stiffer componen
                * gets less velocity). The velocity equation for a pennate muscles is:
                *    Vmt = Vtend + Vfibers / cos(pennation)
                * Using the fiber and tendon stiffnesses to distribute velocity:
                *    Vtend = Vmt * (fiber_stiffness / (fiber_stiffness + tendon_stiffness))
                * Then plugging the second equation into the first:
                *    Vfibers = (1-B) * cos(pennation) * Vmt,
                * where B = (fiber_stiffness / (fiber_stiffness + tendon_stiffness)
                */

            if (j == 1)
                break;

            beta = fiber_stiffness / (fiber_stiffness + tendon_stiffness);
            tendonVelocity_ = muscleTendonVelocity_ * beta;
            fiberVelocity_ = (1.0 - beta) * cf * muscleTendonVelocity_;


        }
        return fiberLength_;

    }
    //*************//


    template <CurveMode::Mode mode>
    ElasticTendon<mode>::ElasticTendon() :
        optimalFiberLength_(.0),
        pennationAngle_(.0),
        tendonSlackLength_(.0),
        percentageChange_(.0),
        damping_(.0),
        maxIsometricForce_(.0),
        strengthCoefficient_(.0),
        maxContractionVelocity_(0.0),
        muscleTendonLength_(0.0),
        muscleTendonLengthT1_(0.0),
        muscleTendonVelocity_(0.0),
        muscleTendonVelocityT1_(0.0),
        fiberLength_(0.0),
        fiberLengthT1_(0.0),
        fiberVelocity_(0.0),
        muscleTendonForce_(0.0),
        activation_(0.0),
        activationT1_(0.0),
        timescale_(0.004),
        nLmt_(0),
        lmtTime_(.0),
        lmtTimeT1_(.0),
        id_("")
    {  }


    template <CurveMode::Mode mode>
    ElasticTendon<mode>::ElasticTendon(std::string id) :
        optimalFiberLength_(.0),
        pennationAngle_(.0),
        tendonSlackLength_(.0),
        percentageChange_(.0),
        damping_(.0),
        maxIsometricForce_(.0),
        strengthCoefficient_(.0),
        maxContractionVelocity_(0.0),
        muscleTendonLength_(0.0),
        muscleTendonLengthT1_(0.0),
        muscleTendonVelocity_(0.0),
        muscleTendonVelocityT1_(0.0),
        fiberLength_(0.0),
        fiberLengthT1_(0.0),
        fiberVelocity_(0.0),
        muscleTendonForce_(0.0),
        activation_(0.0),
        activationT1_(0.0),
        timescale_(0.004),
        nLmt_(0),
        lmtTime_(.0),
        lmtTimeT1_(.0),
        id_(id)
    { }


    template <CurveMode::Mode mode>
    ElasticTendon<mode>::ElasticTendon(double optimalFiberLength,
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
        const CurveOffline& tendonForceStrainCurve) :

        optimalFiberLength_(optimalFiberLength),
        pennationAngle_(pennationAngle),
        tendonSlackLength_(tendonSlackLength),
        percentageChange_(percentageChange),
        damping_(damping),
        maxIsometricForce_(maxIsometricForce),
        strengthCoefficient_(strengthCoefficient),
        maxContractionVelocity_(maxContractionVelocity),
        activeForceLengthCurve_(activeForceLengthCurve),
        passiveForceLengthCurve_(passiveForceLengthCurve),
        forceVelocityCurve_(forceVelocityCurve),
        tendonForceStrainCurve_(tendonForceStrainCurve),
        muscleTendonLength_(0.0),
        muscleTendonLengthT1_(0.0),
        muscleTendonVelocity_(0.0),
        muscleTendonVelocityT1_(0.0),
        fiberLength_(0.0),
        fiberLengthT1_(0.0),
        fiberVelocity_(0.0),
        muscleTendonForce_(0.0),
        activation_(0.0),
        activationT1_(0.0),
        timescale_(0.004),
        nLmt_(0),
        lmtTime_(.0),
        lmtTimeT1_(.0),
        id_("")
    {   }


    template <CurveMode::Mode mode>
    ElasticTendon<mode>::ElasticTendon(const ElasticTendon<mode>& orig)
    {

        cout << "ElasticTendon copy constructor. EXIT\n";
        exit(EXIT_FAILURE);
    }


    template <CurveMode::Mode mode>
    ElasticTendon<mode>& ElasticTendon<mode>::operator= (const ElasticTendon<mode>& orig)
    {

        optimalFiberLength_ = orig.optimalFiberLength_;
        pennationAngle_ = orig.pennationAngle_;
        tendonSlackLength_ = orig.tendonSlackLength_;
        percentageChange_ = orig.percentageChange_;
        damping_ = orig.damping_;
        maxIsometricForce_ = orig.maxIsometricForce_;
        strengthCoefficient_ = orig.strengthCoefficient_;
        maxContractionVelocity_ = orig.maxContractionVelocity_;
        activeForceLengthCurve_ = orig.activeForceLengthCurve_;
        passiveForceLengthCurve_ = orig.passiveForceLengthCurve_;
        forceVelocityCurve_ = orig.forceVelocityCurve_;
        tendonForceStrainCurve_ = orig.tendonForceStrainCurve_;

        muscleTendonLength_ = orig.muscleTendonLength_;
        muscleTendonLengthT1_ = orig.muscleTendonLengthT1_;
        muscleTendonVelocity_ = orig.muscleTendonVelocity_;
        muscleTendonVelocityT1_ = orig.muscleTendonVelocityT1_;
        fiberLength_ = orig.fiberLength_;
        fiberLengthT1_ = orig.fiberLengthT1_;
        fiberVelocity_ = orig.fiberVelocity_;
        muscleTendonForce_ = orig.muscleTendonForce_;
        activation_ = orig.activation_;
        activationT1_ = orig.activationT1_;
        timescale_ = orig.timescale_;
        lmtTime_ = orig.lmtTime_;
        lmtTimeT1_ = orig.lmtTimeT1_;
        nLmt_ = orig.nLmt_;
        id_ = orig.id_;
        return *this;
    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setParametersToComputeForces(double optimalFiberLength,
        double pennationAngle,
        double tendonSlackLength,
        double percentageChange,
        double damping,
        double maxIsometricForce,
        double strengthCoefficient,
        double maxContractionVelocity) {

        optimalFiberLength_ = optimalFiberLength;
        pennationAngle_ = pennationAngle;
        tendonSlackLength_ = tendonSlackLength;
        percentageChange_ = percentageChange;
        damping_ = damping;
        maxIsometricForce_ = maxIsometricForce;
        strengthCoefficient_ = strengthCoefficient;
        maxContractionVelocity_ = maxContractionVelocity;
    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setCurves(const CurveOffline& activeForceLengthCurve,
        const CurveOffline& passiveForceLengthCurve,
        const CurveOffline& forceVelocityCurve,
        const CurveOffline& tendonForceStrainCurve) {


        activeForceLengthCurve_ = activeForceLengthCurve;
        passiveForceLengthCurve_ = passiveForceLengthCurve;
        forceVelocityCurve_ = forceVelocityCurve;
        tendonForceStrainCurve_ = tendonForceStrainCurve;
    };



    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setStrengthCoefficient(double strengthCoefficient) {

        strengthCoefficient_ = strengthCoefficient;
        resetState();

    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setTendonSlackLength(double tendonSlackLength) {

        tendonSlackLength_ = tendonSlackLength;
        resetState();

    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setPennationAngle(double pennationAngle) {

        pennationAngle_ = pennationAngle;
        resetState();
    }



    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setMaxIsometricForce(double maxIsometricForce) {

        maxIsometricForce_ = maxIsometricForce;
        resetState();
    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setMaxContractionVelocity(double maxContractionVelocity) {

        maxContractionVelocity_ = maxContractionVelocity;
        resetState();
    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::resetState() {

        muscleTendonLength_ = 0.0;
        muscleTendonLengthT1_ = 0.0;
        muscleTendonVelocity_ = 0.0;
        muscleTendonVelocityT1_ = 0.0;
        fiberLength_ = 0.0;
        fiberLengthT1_ = 0.0;
        fiberVelocity_ = 0.0;
        muscleTendonForce_ = 0.0;
        activation_ = 0.0;
        activationT1_ = 0.0;
        nLmt_ = 0;
        lmtTime_ = .0;
        lmtTimeT1_ = .0;
        muscleTendonLengthTrace_.reset();
    }


    template <CurveMode::Mode mode>
    double ElasticTendon<mode>::getInitialFiberLength(double time) {


        //   muscleTendonVelocity_ = (muscleTendonLength_ - muscleTendonLengthT1_)/timescale_;

        LDFM ldfm(optimalFiberLength_,
            pennationAngle_,
            tendonSlackLength_,
            percentageChange_,
            damping_,
            maxIsometricForce_,
            strengthCoefficient_,
            activeForceLengthCurve_,
            passiveForceLengthCurve_,
            forceVelocityCurve_);



        ldfm.setInitConditions(muscleTendonLength_, muscleTendonVelocity_, activation_);

        /* the LDFEM (Loan Dynamic Force Estimation Method) loop is done twice. The
        * first time through, the tendon velocity is assumed to be zero, so all of
        * the velocity is `in' the muscle. The muscle force is then found iteratively
        * for this situation. This is done by guessing the length of the muscle and
        * of the tendon, and calculating their respective forces. If the forces
        * match (are within MAXIMUMERROR of each other), stop; else change the length
        * guesses based on the error and try again. Once the muscle force is known,
        * a better estimate of its velocity can be made. The slope of the force-length
        * curve at this length is compared to the slope of the tendon force-length
        * curve, and the velocities are guessed again.
        */



        /* if the first iteration, adjust the guess of muscle and tendon
        * velocity now that you know the (quasi) real lengths.
        * The total musculotendon velocity is distributed between the fibers
        * and tendon based on their relative stiffnesses (the stiffer component
        * gets less velocity). The velocity equation for a pennate muscles is:
        *    Vmt = Vtend + Vfibers / cos(pennation)
        * Using the fiber and tendon stiffnesses to distribute velocity:
        *    Vtend = Vmt * (fiber_stiffness / (fiber_stiffness + tendon_stiffness))
        * Then plugging the second equation into the first:
        *    Vfibers = (1-B) * cos(pennation) * Vmt,
        * where B = (fiber_stiffness / (fiber_stiffness + tendon_stiffness)
        */

        fiberLength_ = ldfm.estimateFiberLength();

        return fiberLength_;
    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::setMuscleTendonLength(double muscleTendonLength, double activation, double time) {



        lmtTimeT1_ = lmtTime_;
        lmtTime_ = time;
        timescale_ = lmtTime_ - lmtTimeT1_;
        activationT1_ = activation_;
        activation_ = activation;


        muscleTendonLengthT1_ = muscleTendonLength_;
        muscleTendonLength_ = muscleTendonLength;
        muscleTendonLengthTrace_.addPoint(time, muscleTendonLength_);

        muscleTendonVelocityT1_ = muscleTendonVelocity_;
        muscleTendonVelocity_ = muscleTendonLengthTrace_.getFirstDerivative(time);
        //muscleTendonVelocity_ = (muscleTendonLength_ - muscleTendonLengthT1_)/timescale_;

        fiberLengthT1_ = fiberLength_;


        //we need the first two values of lmt for getting the initial muscleTendonVelocity value
        //TODO if nLmt_ == 0 do nothing or something like that, verify which one is better
        if (nLmt_ < 3)
            fiberLength_ = getInitialFiberLength(time);
        else
            fiberLength_ = getFiberLengthRKF();

        ++nLmt_;
    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::updateFiberLengthUsingNewActivation(double activation, double time) {

        activation_ = activation;

        //we need the first two values of lmt for getting the initial muscleTendonVelocity value
        //TODO if nLmt_ == 0 do nothing or something like that, verify which one is better

        if (nLmt_ < 3)
            fiberLength_ = getInitialFiberLength(time);
        else
            fiberLength_ = getFiberLengthRKF();

        ++nLmt_;

    }


    template <CurveMode::Mode mode>
    double ElasticTendon<mode>::getFiberLengthRKF() {


        const int nvar = 1;
        double atol = 1.0e-3, rtol = atol, h1 = 0.01, hmin = 0.0, x1 = lmtTimeT1_, x2 = lmtTime_;
        vector<double> ystart(nvar);
        Output out(1);

        ystart.at(0) = fiberLengthT1_;

        Odeint< StepperDopr5<ElasticTendon> > ode(ystart, x1, x2, atol, rtol, h1, hmin, out, *this);

        ode.integrate();

        //    cout << "rk " << out.ySave_.at(0).at(1);

        return out.ySave_.at(0).at(1);

    }


    template <CurveMode::Mode mode>
    void ElasticTendon<mode>::operator() (double x, const std::vector<double>& y, std::vector<double>& dydx) {

        testTime_ = x;
        double time = x;
        double delta = time - lmtTimeT1_;
        //double lmtTimescale = lmtTime_ - lmtTimeT1_; //should be equal to timescale_

        //**the following can be done with a spline
        double muscleTendonLengthAtT = (delta *(muscleTendonLength_ - muscleTendonLengthT1_) / timescale_) + muscleTendonLengthT1_;
        double activationAtT = (delta *(activation_ - activationT1_) / timescale_) + activationT1_;
        double muscleTendonVelocityAtT = (delta *(muscleTendonVelocity_ - muscleTendonVelocityT1_) / timescale_) + muscleTendonVelocityT1_;

        dydx.at(0) = computeFiberVelocityAtT(muscleTendonLengthAtT, activationAtT, muscleTendonVelocityAtT, y.at(0), timescale_);

    }



    //this is the function to integrate
    template <CurveMode::Mode mode>
    double ElasticTendon<mode>::computeFiberVelocityAtT(double muscleTendonLengthAtT, double activationAtT, double muscleTendonVelocityAtT, double fiberLength, double lmtTimescale) {


        double optimalFiberLengthAtT = optimalFiberLength_ * (percentageChange_ * (1.0 - activationAtT) + 1.0);
        double normFiberLength = fiberLength / optimalFiberLengthAtT;
        double pennationAngleAtT = ceinms::PennationAngle::compute(fiberLength, optimalFiberLength_, pennationAngle_);
        double ca = cos(radians(pennationAngleAtT));
        double tendonLength = muscleTendonLengthAtT - fiberLength*ca;
        double tendonStrain = (tendonLength - tendonSlackLength_) / tendonSlackLength_;
        double normTendonForce = tendonForceStrainCurve_.getValue(tendonStrain);
        double fa = activeForceLengthCurve_.getValue(normFiberLength);

        double normFiberVelocity = 0.0;
        double velocityDependentForce = 0.0;

        /* If pennation equals 90 degrees, fiber length equals muscle width and fiber
        * velocity goes to zero.  Pennation will stay at 90 until tendon starts to
        * pull, then "stiff tendon" approximation is used to calculate approximate
        * fiber length.
        */

        if ((tendonLength < tendonSlackLength_) && (ca> 0)) {
            //      cout << id_ << " tendonLength < tendonSlackLength " << endl << tendonLength << " " << tendonSlackLength_ << endl;
            double first = optimalFiberLengthAtT * sin(radians(pennationAngleAtT));
            double second = muscleTendonLength_ - tendonSlackLength_;
            double fiberLength = sqrt(first * first + second * second);
            pennationAngleAtT = ceinms::PennationAngle::compute(fiberLength, optimalFiberLengthAtT, pennationAngle_);
            ca = cos(radians(pennationAngleAtT));
            normFiberVelocity = lmtTimescale*(muscleTendonVelocityAtT / optimalFiberLengthAtT) * ca;
#ifdef DEBUG
            //            if(id_ == "gaslat_r")
            {
                cout << id_ << " computeFiberVelocityAtT@Z1 \n";
                //                cout << "tendonlength " << tendonLength << endl;
            }
#endif

            //        normFiberVelocity = 100;
        }


        else if (fabs(ca) < 0.1) {
            if (fabs(normTendonForce) < 0.1)  // 0.01 means close to zero, could be changed
            {
#ifdef DEBUG
                //              if(id_ == "gaslat_r")
                cout << id_ << " computeFiberVelocityAtT@Z2 \n";
#endif
                normFiberVelocity = 0.0;
            }
            else {
                double first = optimalFiberLengthAtT * sin(radians(pennationAngleAtT));
                double second = muscleTendonLength_ - tendonSlackLength_;
                double fiberLength = sqrt(first * first + second * second);
                pennationAngleAtT = ceinms::PennationAngle::compute(fiberLength, optimalFiberLengthAtT, pennationAngle_);
                ca = cos(radians(pennationAngleAtT));
                normFiberVelocity = lmtTimescale*(muscleTendonVelocityAtT / optimalFiberLengthAtT) * ca;
#ifdef DEBUG
                //              if(id_ == "gaslat_r")
                cout << id_ << " computeFiberVelocityAtT@Z3 \n";
#endif
            }
        }
        else {
            double fp = passiveForceLengthCurve_.getValue(normFiberLength);
            velocityDependentForce = normTendonForce / ca - fp;
            normFiberVelocity = computeNormFiberVelocity(activationAtT, fa, velocityDependentForce);
#ifdef DEBUG
            //              if(id_ == "gaslat_r")
            cout << id_ << " computeFiberVelocityAtT@Z4 \n";
            //                cout << "activationAtT, fa, velocityDependentForce, pennationAngleAtT\n";
            //                cout << activationAtT << " " << fa << " " << velocityDependentForce << " " << pennationAngleAtT <<  endl;
#endif
        }
        /* Un-normalize the muscle state derivatives */
#ifdef DEBUG
        /*          if(id_ == "bicfemsh_r") {
            cout << testTime_ << "\nnormFiberVelocity optimalFiberLengthAtT lmtTimescale\n";
            cout << normFiberVelocity << " " << optimalFiberLengthAtT  << " " << lmtTimescale << endl;
            cout << (normFiberVelocity * optimalFiberLengthAtT / lmtTimescale) << endl;}*/
#endif
        return (normFiberVelocity * optimalFiberLengthAtT / lmtTimescale);

    }


    template <CurveMode::Mode mode>
    double ElasticTendon<mode>::computeNormFiberVelocity(double activation, double activeForce, double velocityDependentForce) {

        double b, c, fiberVelocity;

        double kv = 0.25, slope_k = 0.13, fmax = 1.8;

        //this equation are in Schutte phd thesis.

        //    if (velocityDependentForce < -damping_)
        //        fiberVelocity = velocityDependentForce / damping_;
        if (velocityDependentForce < activation*activeForce) {
            c = kv * (velocityDependentForce - activation*activeForce) / damping_;
            b = -kv * (velocityDependentForce / kv + activation*activeForce + damping_) / damping_;
            //      cout << "a. " << b*b-4.0*c << endl;
            fiberVelocity = (-b - sqrt(b*b - 4.0*c)) / 2.0;

        }
        else {
            c = -(slope_k*kv / ((damping_*(kv + 1))))* (velocityDependentForce - activation*activeForce);
            b = -(velocityDependentForce / damping_ - fmax*activation*activeForce / damping_ - slope_k*kv / (kv + 1));

            //     cout << "b. " << b*b-4.0*c << endl;
            // fiber_velocity = (-b+sqrt(b*b-4.0*c)) / 2.0;

            fiberVelocity = -2 * c / (b + sqrt(b*b - 4.0*c));

        }

        // cout << "activation " << activation << " activeForce " << activeForce << " velocityDependentForce " << velocityDependentForce << endl;
        return fiberVelocity;
    }

    template <CurveMode::Mode mode>
    ElasticTendon<mode>::~ElasticTendon() { }
}
