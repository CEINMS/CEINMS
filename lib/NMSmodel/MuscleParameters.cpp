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

#include <limits>
#include "MuscleParameters.h"

static const double dblLow(std::numeric_limits<double>::lowest());

namespace ceinms {

    MuscleParameters::MuscleParameters() :
        c1_(std::numeric_limits<double>::lowest()), c2_(dblLow), shapeFactor_(dblLow),
        optimalFiberLength_(dblLow), pennationAngle_(dblLow),
        tendonSlackLength_(dblLow), maxIsometricForce_(dblLow),
        strengthCoefficient_(dblLow), emDelay_(dblLow)
    {

    }

    bool MuscleParameters::operator==(const MuscleParameters &other) const {

        return  c1_ == other.c1_                                    &&
                c2_ == other.c2_                                    &&
                shapeFactor_ == other.shapeFactor_                  &&
                optimalFiberLength_ == other.optimalFiberLength_    &&
                pennationAngle_ == other.pennationAngle_            &&
                tendonSlackLength_ == other.tendonSlackLength_      &&
                maxIsometricForce_ == other.maxIsometricForce_      &&
                strengthCoefficient_ == other.strengthCoefficient_  &&
                maxContractionVelocity_ == other.maxContractionVelocity_ &&
                emDelay_ == other.emDelay_;
    }


    MuscleParameters::~MuscleParameters()
    {

    }
}
