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
* Author(s): E. Ceseracciu, C. Pizzolato, M. Reggiani                        *
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

#include <memory>
using std::shared_ptr;
#include <string>
using std::string;
#include "ceinms/LmtMaFromMcbs.h"
#include <vector>
using std::vector;
#include <algorithm>
#include "ceinms/EndOfData.h"

namespace ceinms {
    
    shared_ptr<SplineSetBase> makeSplineSet(int degree, const string& inputDataFilename) {
   
        std::function<shared_ptr<SplineSetBase>(const string&)> ftable[] = {
            nullptr,
            nullptr,
            &wrap<2, string>,
            &wrap<3, string>,
            &wrap<4, string>,
            &wrap<5, string>,
            &wrap<6, string>,
            &wrap<7, string>,
        };

        if (degree > 7) {
            std::cout << "Degree of mcbs too high.\nExit.\n";
            std::exit(EXIT_FAILURE);
        }
        return ftable[degree](inputDataFilename);
    }
    
    
    shared_ptr<SplineSetBase> makeSplineSet(int degree,
        const vector<string > dofNames,
        const vector<string > musclesNames,
        const vector<double>& a,
        const vector<double>& b,
        const vector<int>& n,
        const vector<vector<double>>& y) {

        std::function<shared_ptr<SplineSetBase>(
            const vector<string >,
            const vector<string >,
            const vector<double>&,
            const vector<double>&,
            const vector<int>&,
            const vector<vector<double> >&)> ftable[] = {
            
            nullptr,
            nullptr,
            &wrapAlias<2>,
            &wrapAlias<3>,
            &wrapAlias<4>,
            &wrapAlias<5>,
            &wrapAlias<6>,
            &wrapAlias<7>,
        };

        if (degree > 7) {
            std::cout << "Degree of mcbs too high.\nExit.\n";
            std::exit(EXIT_FAILURE);
        }
        return ftable[degree](dofNames, musclesNames, a, b, n, y);
    }
    

    shared_ptr<SplineSetBase> makeSplineSet(
        int degree,
        const string& coeffDir,
        const vector<string>& muscleNames) {

        std::function<shared_ptr<SplineSetBase>(
            const string&, 
            const vector<string>&)> ftable[] = {
            nullptr,
            nullptr,
            &wrap<2, const string&, const vector<string>&>,
            &wrap<3, const string&, const vector<string>&>,
            &wrap<4, const string&, const vector<string>&>,
            &wrap<5, const string&, const vector<string>&>,
            &wrap<6, const string&, const vector<string>&>,
            &wrap<7, const string&, const vector<string>&>,
        };

        if (degree > 7) {
            std::cout << "Degree of mcbs too high.\nExit.\n";
            std::exit(EXIT_FAILURE);
        }
        return ftable[degree](coeffDir, muscleNames);
    }
    
 
    void LmtMaFromMcbs::operator()() {

        inputCoordinateQueue_.subscribe();
        LmtMaFromX::doneWithSubscriptions();

        bool runCondition(true);

        do {
            GeneralisedCoordinatesFrame coordinates(inputCoordinateQueue_.pop());
            double currentTime(coordinates.time);
            if (EndOfData::isEod(coordinates)) {
                LmtData lmts;
                vector<double> dofsValues(dofMapper_.map(coordinates.data.getQ()));

                splineSet_->getLmt(lmts, dofsValues);
                LmtMaFromX::updateLmt(lmts, currentTime);
                for (auto dofIt(dofNames_.cbegin()); dofIt != dofNames_.cend(); ++dofIt) {
                    MomentArmsData maValues;
                    splineSet_->getMa(*dofIt, muscleNamesOnDofsMap_[*dofIt], dofsValues, maValues);
                    unsigned dofIdx(std::distance(dofNames_.cbegin(), dofIt));
                    LmtMaFromX::updateMomentArms(maValues, currentTime, dofIdx);
                }
            }
            else {
                runCondition = false;
                LmtMaFromX::sendLmtEndOfData();
                LmtMaFromX::sendMaEndOfData();
            }

        } while (runCondition);
        LmtMaFromX::doneWithExecution();

    }


}