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

#ifndef ceinms_KneeBalanceContactModel_h
#define ceinms_KneeBalanceContactModel_h
#include "ceinms/ContactModelKneeBalanceSetup.h"
#include "ceinms/Mapper.h"
#include <vector>
#include <string>

namespace ceinms {

    class KneeBalanceContactModel {

    public:
        KneeBalanceContactModel();

        KneeBalanceContactModel(
            const ContactModelKneeBalanceSetup& setup,
            const std::vector<std::string>& dofNamesFromSubject);

        template<typename NMSmodelT>
        KneeBalanceContactModel(
            const ContactModelKneeBalanceSetup& setup,
            const NMSmodelT& subject);

        void initialize(const ContactModelKneeBalanceSetup& setup, const std::vector<std::string>& names);
        // externalTorques and estimatedTorques should have length of dofNamesFromSubject,
        // the mapping is done internally
        void setTorques(const std::vector<double>& externalTorques, const std::vector<double>& estimatedTorques);
        void getCondyleNames(std::vector<std::string>& names) const { names = condyleNames_; }
        void getContactForces(std::vector<double>& forces) const { forces = contactForces_; }
        std::vector<std::string> getCondyleNames() const { return condyleNames_; }
        std::vector<double> getContactForces() const { return contactForces_; }

    private:
        void calculateContactForces(const std::vector<double>& externalTorques, const std::vector<double>& estimatedTorques);
        double calculateSingleForce(double externalTorque, double estimatedTorque) const;
        double intercondyleDistance_;
        std::vector<double> contactForces_;
        std::vector<std::string> condyleNames_, dofNamesFromSubject_;
        std::vector<Mapper> fromSubjectTorquesToCondyleOppositeTorque_;
    };

    template<typename NMSmodelT>
    KneeBalanceContactModel::KneeBalanceContactModel(
        const ContactModelKneeBalanceSetup& setup,
        const NMSmodelT& subject) :
        fromSubjectTorquesToCondyleOppositeTorque_(2),
        condyleNames_(2),
        contactForces_(2){

        std::vector<std::string> dofNamesFromSubject;
        subject.getDoFNames(dofNamesFromSubject);
        initialize(setup, dofNamesFromSubject);
    }
}

#endif