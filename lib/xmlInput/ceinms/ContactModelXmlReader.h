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

#ifndef ceinms_ContactModelXmlReader_h
#define ceinms_ContactModelXmlReader_h

#include <string>
#include <vector>

#include "contactModel.hxx"
#include "ceinms/ContactModelFromOpenSimSetup.h"
#include "ceinms/ContactModelKneeBalanceSetup.h"

namespace ceinms {

    //in future we could have different sources..
    enum class ContactModelType { OpenSim, KneeBalance };

    class ContactModelXmlReader {
    public:
        ContactModelXmlReader();
        ContactModelXmlReader(const std::string& filename);
        void setXmlInputFile(const std::string& filename);
        ContactModelType getContactModelType() const;
        ContactModelFromOpenSimSetup getContactModelSetupOsim() const;
        ContactModelKneeBalanceSetup getContactModelSetupKneeBalance() const;

    private:
        void init();
        void readXml();
        void checkFile(const std::string& filename) const;
        std::string getCompleteFilename(std::string filename) const;
        ContactModelFromOpenSimSetup setupOsim_;
        ContactModelKneeBalanceSetup setupKneeBalance_;
        std::unique_ptr<ContactModelXsd::ContactModelType> contactModelPointer_;
        ContactModelType contactModelType_;
        std::string filepath_;
        std::string filename_;
    };
}

#endif