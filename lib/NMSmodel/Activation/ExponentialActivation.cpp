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

#include "ExponentialActivation.h"
#include <cmath>

namespace ceinms {

    ExponentialActivation::ExponentialActivation() :
        beta1_(0.0), beta2_(0.0), alpha_(0.0), shapeFactor_(0.0),
        emg_(0.0), noOfEmg_(0), pastEmgT1_(0.0), pastEmgT2_(0.0), neuralActivation_(0.0),
        neuralActivationT1_(0.0), neuralActivationT2_(0.0), activation_(0.0), expShapeFactor_(0.0)

    {

    }

    void ExponentialActivation::setFilterParameters(double c1, double c2) {

        beta1_ = c1 + c2;
        beta2_ = c1 * c2;
        alpha_ = 1 + beta1_ + beta2_;
    }


    void ExponentialActivation::setShapeFactor(double shapeFactor) {

        shapeFactor_ = shapeFactor;
        expShapeFactor_ = exp(shapeFactor_);
    }

    bool ExponentialActivation::setEmg(double emg) {

        emg_ = emg;
        if (emg_ > 1)
        {
            emg_ = 1;
            return false;
        }
        if (emg_ < 0)
        {
            emg_ = 0;
            return false;
        }
        return true;
    }


    void ExponentialActivation::updateActivation() {

        if (noOfEmg_ == 2) {
            neuralActivationT1_ = (pastEmgT2_ + pastEmgT1_) / 2;
            neuralActivationT2_ = neuralActivationT1_;
        }

        neuralActivation_ = (alpha_*emg_) - (beta1_*neuralActivationT1_) - (beta2_*neuralActivationT2_);
        activation_ = (exp(shapeFactor_*neuralActivation_) - 1) / (expShapeFactor_ - 1);
    }


    void ExponentialActivation::pushState() {

        neuralActivationT2_ = neuralActivationT1_;
        neuralActivationT1_ = neuralActivation_;
        pastEmgT2_ = pastEmgT1_;
        pastEmgT1_ = emg_;
        noOfEmg_++;
    }


    void ExponentialActivation::resetState() {

        emg_ = .0;
        activation_ = .0;
        neuralActivation_ = .0;
        neuralActivationT1_ = .0;
        neuralActivationT2_ = .0;
        pastEmgT1_ = .0;
        pastEmgT2_ = .0;
        noOfEmg_ = 0;
    }


    ExponentialActivation::~ExponentialActivation() { }

}
