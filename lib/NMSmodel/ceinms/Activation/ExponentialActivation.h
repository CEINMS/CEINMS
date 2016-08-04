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
 * Author(s): C. Pizzolato, M. Reggiani, D.G. Lloyd                           *
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

#ifndef ceinms_ExponentialActivation_h
#define ceinms_ExponentialActivation_h

namespace ceinms {
    class ExponentialActivation {

    public:
        ExponentialActivation();
        virtual ~ExponentialActivation();

        void   setShapeFactor(double shapeFactor);
        void   setFilterParameters(double c1, double c2);
        bool   setEmg(double emg);
        void   updateActivation();
        void   resetState();
        void   pushState();
        double getActivation() const { return activation_; }
        double getEmg() const { return emg_; }
        double getPastEmg() const { return pastEmgT1_; } //controllare se viene usata ancora 7Mar13
        double getNeuralActivation() const { return neuralActivation_; }

    private:
        double beta1_;         /**< \f$\beta_1\f$ recursive coefficient */
        double beta2_;         /**< \f$\beta_1\f$ recursive coefficient */
        double alpha_;         /**< \f$\alpha\f$ gain coefficient */
        double shapeFactor_;   /**< \f$A\f$ non-linear shape factor */
        double expShapeFactor_; //   exp(shapeFactor)
        double emg_;           /**< \f$e(t)\f$ high-pass filtered, full-wave rectified,
                                *   and low-pass filtered EMG
                                */
        unsigned noOfEmg_;
        double pastEmgT1_;
        double pastEmgT2_;
        double neuralActivation_;   /**< \f$u(t)\f$ post-processed EMG of muscle at time t */
        double neuralActivationT1_; /**< \f$u(t-1)\f$ post-processed EMG of muscle at time t-1 */
        double neuralActivationT2_; /**< \f$u(t-2)\f$ post-processed EMG of muscle at time t-2 */
        double activation_;    /**< \f$a(t)\f$ activation of the muscle */
    };
}

#endif
