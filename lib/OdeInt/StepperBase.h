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
 
#ifndef Stepperbase_h
#define StepperBase_h

#include <vector>

class StepperBase {
public:
    StepperBase(std::vector<double>& y, std::vector<double>& dydx, double& x, const double aTol,
                const double rTol, bool dense) : x_(x), y_(y), dydx_(dydx), aTol_(aTol),
    rTol_(rTol), dense_(dense), n_(y.size()) ,nEqn_(y.size()), yOut_(y.size()),yErr_(y.size()) {}

    double& x_;
    double xOld_;
    std::vector<double>& y_;
    std::vector<double>& dydx_;
    double aTol_, rTol_;
    bool dense_;
    double hDid_;
    double hNext_;
    double EPS_;
    int n_, nEqn_;
    std::vector<double> yOut_, yErr_;
};

#endif
