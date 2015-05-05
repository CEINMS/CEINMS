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

#ifndef ceinms_NMSmodelPolicyTemplates_h
#define ceinms_NMSmodelPolicyTemplates_h

#include <iostream>
#include "ExponentialActivation.h"
#include "PiecewiseActivation.h"
#include "StiffTendon.h"
#include "ElasticTendon.h"
#include "ElasticTendon_BiSec.h"
#include "Curve.h"

namespace ceinms {
    template class NMSmodel < ExponentialActivation, StiffTendon, CurveMode::Offline > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline>& b);

    template class NMSmodel < ExponentialActivation, StiffTendon, CurveMode::Online > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>& b);

    template class NMSmodel < PiecewiseActivation, StiffTendon, CurveMode::Offline > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline>& b);

    template class NMSmodel < PiecewiseActivation, StiffTendon, CurveMode::Online > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>& b);



    // template class NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>;
    // template
    // std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>& b);
    //
    // template class NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Online>, CurveMode::Online>;
    // template
    // std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Online>& b);
    //
    // template class NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>;
    // template
    // std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>& b);
    //
    // template class NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Online>;
    // template
    // std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Online>& b);
    //



    template class NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline>& b);

    template class NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>& b);

    template class NMSmodel < PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline>& b);

    template class NMSmodel < PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    template
        std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>& b);
}

#endif
