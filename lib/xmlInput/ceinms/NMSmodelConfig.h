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

#ifndef ceinms_NMSModelCfg_h
#define ceinms_NMSModelCfg_h

namespace ceinms {
    namespace NMSModelCfg {

        const unsigned OpenLoop = 1000;
        const unsigned Hybrid = 2000;

        const unsigned ExponentialActivation = 100;
        const unsigned PiecewiseActivation = 200;

        const unsigned StiffTendon = 10;
        const unsigned ElasticTendon = 20;
        const unsigned ElasticTendonBiSec = 30;

        const unsigned Online = 1;
        const unsigned Offline = 2;

        const unsigned Nop = 0;


        enum RunMode{

            OpenLoopExponentialActivationStiffTendonOnline = 1111,
            OpenLoopExponentialActivationStiffTendonOffline = 1112,
            OpenLoopExponentialActivationElasticTendonOnline = 1121,
            OpenLoopExponentialActivationElasticTendonOffline = 1122,
            OpenLoopExponentialActivationElasticTendonBiSecOnline = 1131,
            OpenLoopExponentialActivationElasticTendonBiSecOffline = 1132,

            StiffnessOpenLoopExponentialActivationElasticTendonBiSecOffline = 11132,

            OpenLoopPiecewiseActivationStiffTendonOnline = 1211,
            OpenLoopPiecewiseActivationStiffTendonOffline = 1212,
            OpenLoopPiecewiseActivationElasticTendonOnline = 1221,
            OpenLoopPiecewiseActivationElasticTendonOffline = 1222,
            OpenLoopPiecewiseActivationElasticTendonBiSecOnline = 1231,
            OpenLoopPiecewiseActivationElasticTendonBiSecOffline = 1232,

            HybridExponentialActivationStiffTendonOnline = 2111,
            HybridExponentialActivationStiffTendonOffline = 2112,
            HybridExponentialActivationElasticTendonOnline = 2121,
            HybridExponentialActivationElasticTendonOffline = 2122,
            HybridExponentialActivationElasticTendonBiSecOnline = 2131,
            HybridExponentialActivationElasticTendonBiSecOffline = 2132,

            HybridPiecewiseActivationStiffTendonOnline = 2211,
            HybridPiecewiseActivationStiffTendonOffline = 2212,
            HybridPiecewiseActivationElasticTendonOnline = 2221,
            HybridPiecewiseActivationElasticTendonOffline = 2222,
            HybridPiecewiseActivationElasticTendonBiSecOnline = 2231,
            HybridPiecewiseActivationElasticTendonBiSecOffline = 2232,
        };
    };
}

#endif
