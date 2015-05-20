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

#include "ceinms/KneeBalanceContactModel.h"
#include <vector>
using std::vector;
#include <string>
using std::string;

namespace ceinms {

    KneeBalanceContactModel::KneeBalanceContactModel():
        fromSubjectTorquesToCondyleOppositeTorque_(2),
        condyleNames_(2), contactForces_(2){}

    KneeBalanceContactModel::KneeBalanceContactModel(const ContactModelKneeBalanceSetup& setup, const vector<string>& dofNamesFromSubject):
        fromSubjectTorquesToCondyleOppositeTorque_(2),
        condyleNames_(2), contactForces_(2) {
        initialize(setup, dofNamesFromSubject);
    }

    void KneeBalanceContactModel::initialize(const ContactModelKneeBalanceSetup& setup, const vector<string>& dofNamesFromSubject) {

        intercondyleDistance_ = setup.intercondyleDistance;
        condyleNames_.at(0) = setup.condyle1.name;
        condyleNames_.at(1) = setup.condyle2.name;
        
        condyleNames_.at(0) = setup.condyle1.name;
        condyleNames_.at(1) = setup.condyle2.name;

        //for each condyle you have to use the opposite external and estimated moments
        fromSubjectTorquesToCondyleOppositeTorque_.at(0).setNames(dofNamesFromSubject_, { condyleNames_.at(1) });
        fromSubjectTorquesToCondyleOppositeTorque_.at(1).setNames(dofNamesFromSubject_, { condyleNames_.at(0) });

    }

    void KneeBalanceContactModel::setTorques(const vector<double>& externalTorques, const vector<double>& estimatedTorques) {

        calculateContactForces(externalTorques, estimatedTorques);

    }

    void KneeBalanceContactModel::calculateContactForces(const vector<double>& externalTorques, const vector<double>& estimatedTorques) {

        contactForces_.at(0) =  calculateSingleForce(
            fromSubjectTorquesToCondyleOppositeTorque_.at(0).map(externalTorques).front(),
            fromSubjectTorquesToCondyleOppositeTorque_.at(0).map(estimatedTorques).front());

        contactForces_.at(1) = calculateSingleForce(
            fromSubjectTorquesToCondyleOppositeTorque_.at(1).map(externalTorques).front(),
            fromSubjectTorquesToCondyleOppositeTorque_.at(1).map(estimatedTorques).front());
        
    }

    double KneeBalanceContactModel::calculateSingleForce(double externalTorque, double estimatedTorque) const {

        return (estimatedTorque - externalTorque) / intercondyleDistance_;
    }

    
}