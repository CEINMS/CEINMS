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

#include "ceinms/ContactModelXmlReader.h"
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <list>
using std::list;

#include "ceinms/FileUtils.h"
#include "contactModel-schema.hxx"
#include "validation.h"

namespace ceinms {

    ContactModelXmlReader::ContactModelXmlReader(const string& filename) {

        try {
            std::auto_ptr<ContactModelXsd::ContactModelType> contactModelPointer(parseAndValidate<ContactModelXsd::ContactModelType>(filename, contactModel_schema, sizeof(contactModel_schema)));
            contactModelPointer_ = contactModelPointer;
        }
        catch (const xml_schema::exception& e) {
            cout << e << endl;
            exit(EXIT_FAILURE);
        }

        // save the location of input filename
        string::size_type dirSep = filename.rfind('/'); // Unix/Mac dir separator

        if (dirSep == string::npos)
            dirSep = filename.rfind('\\'); // DOS dir separator

        if (dirSep != string::npos) // if '_fileName' contains path information...
            filepath_ = filename.substr(0, dirSep + 1); // include trailing slashes

        readXml();
    }

    ContactModelType ContactModelXmlReader::getContactModelType() const {

        return ContactModelType::OpenSim;
    }

    void ContactModelXmlReader::readXml() {
        contactModelPointer_->fromOpenSim();
        ContactModelXsd::FromOpenSimType& fromOsim(*(contactModelPointer_->fromOpenSim().begin()));
        setup_.osimModel = getCompleteFilename(*(fromOsim.osimModel().begin()));
        setup_.motion = getCompleteFilename(*(fromOsim.motion().begin()));
        setup_.externalLoads = getCompleteFilename(*(fromOsim.externalLoads().begin()));
        
        ContactModelXsd::JointsType& jointXsd(*fromOsim.joints().begin());
        for (ContactModelXsd::JointsType::iterator it = jointXsd.begin(); it != jointXsd.end(); ++it)
            setup_.joints.push_back(*it);
    }

    string ContactModelXmlReader::getCompleteFilename(string filename) const {

        FileUtils::trim(filename);
        if (FileUtils::isAbsolute(filename.c_str()))
            return filename;
        else
            return filepath_ + filename;
    }


    ContactModelFromOpenSimSetup ContactModelXmlReader::getContactModelSetupOsim() const {
        
        return setup_;

    }


}