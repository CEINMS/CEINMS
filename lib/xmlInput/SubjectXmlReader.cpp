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
 * Author(s): E. Ceseracciu                                                   *
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

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

#include "ceinms/SubjectXmlReader.h"
#include "ceinms/FileUtils.h"
#include "validation.h"
#include "subject-schema.hxx"
using SubjectXsd::SubjectType;

namespace ceinms {
    SubjectXmlReader::SubjectXmlReader(const std::string& filename)
    {
        std::cout << "Reading subject file: " << filename << " ." << std::endl;
        try {
            subjectPointer_ = parseAndValidate<SubjectType>(filename, subject_schema, sizeof(subject_schema));
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

    }

    std::auto_ptr<SubjectType> SubjectXmlReader::getSubject()
    {
        return subjectPointer_;
    }

    std::string SubjectXmlReader::getOpenSimModelFile() const {
    

        std::string fileName(subjectPointer_->opensimModelFile().get());
        if (fileName.empty())
            return "";
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return FileUtils::simplify(filepath_ + fileName);
    }
    
    

    bool SubjectXmlReader::hasOpenSimModel() const {

        return subjectPointer_->opensimModelFile().present();

    }


    
    std::string SubjectXmlReader::getContactModelFile() const{


        std::string fileName(subjectPointer_->contactModelFile().get());
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return FileUtils::simplify(filepath_ + fileName);
    }

    bool SubjectXmlReader::hasContactModel() const {

        return subjectPointer_->contactModelFile().present();
    }
    
}
