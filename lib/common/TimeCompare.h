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
 
#ifndef ceinms_TimeCompare_h
#define ceinms_TimeCompare_h

#include <limits>
#include <cmath>
#define COMPARE_TOLERANCE 0.0001

//TODO: make a proper Time class
namespace ceinms {
    namespace TimeCompare {

        bool equal(double a, double b) {

            //        return fabs(a - b) <= std::numeric_limits<double>::epsilon();
            return fabs(a - b) <= COMPARE_TOLERANCE;
        }

        bool lessEqual(double a, double b) {

            return equal(a, b) || a < b;
        }


        bool less(double a, double b) {
            // a has to be less than b, and not in the tolerance range
            return !equal(a, b) && a < b;
        }

    }
}
#endif
