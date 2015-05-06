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
 * Author(s): C. Pizzolato                                                    *
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

//TODO: this should be in MTU class
#ifndef ceinms_MuscleParameters_h
#define ceinms_MuscleParameters_h

namespace ceinms {
    class MuscleParameters {
    public:
        MuscleParameters();
        ~MuscleParameters();
        bool operator==(const MuscleParameters &other) const;
        double getC1()  const                    { return c1_; }
        double getC2()  const                    { return c2_; }
        double getShapeFactor() const            { return shapeFactor_; }
        double getOptimalFiberLength() const     { return optimalFiberLength_; }
        double getPennationAngle() const         { return pennationAngle_; }
        double getTendonSlackLenght() const      { return tendonSlackLength_; }
        double getMaxIsometricForce() const      { return maxIsometricForce_; }
        double getStrengthCoefficient() const    { return strengthCoefficient_; }
        double getEmDelay() const                { return emDelay_; }

        void setC1(double c1)                                   { c1_ = c1; }
        void setC2(double c2)                                   { c2_ = c2; }
        void setShapeFactor(double shapeFactor)                 { shapeFactor_ = shapeFactor; }
        void setOptimalFiberLength(double optimalFiberLength)   { optimalFiberLength_ = optimalFiberLength; }
        void setPennationAngle(double pennationAngle)           { pennationAngle_ = pennationAngle; }
        void setTendonSlackLength(double tendonSlackLength)     { tendonSlackLength_ = tendonSlackLength; }
        void setMaxIsometricForce(double maxIsometricForce)     { maxIsometricForce_ = maxIsometricForce; }
        void setStrengthCoefficient(double strengthCoefficient) { strengthCoefficient_ = strengthCoefficient; }
        void setEmDelay(double emDelay)                         { emDelay_ = emDelay; }

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
}

#endif
