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
#include <map>
using std::map;
#include <iostream>
using std::cout;
using std::endl;

#include "ceinms/CeinmsCalibrationSetupXmlReader.h"
#include "validation.h"
#include "ceinmsCalibrationSetup-schema.hxx"
using CeinmsCalibrationSetupXsd::CeinmsCalibrationType;

#include "ceinms/FileUtils.h"

namespace ceinms {
    CeinmsCalibrationSetupXmlReader::CeinmsCalibrationSetupXmlReader(const string& filename)
    {

        try {
            std::auto_ptr<CeinmsCalibrationType> ceinmsCalibrationDataPointer(parseAndValidate<CeinmsCalibrationType>(filename, ceinmsCalibrationSetup_schema, sizeof(ceinmsCalibrationSetup_schema)));
            ceinmsCalibrationSetupPointer_ = ceinmsCalibrationDataPointer;
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

    void CeinmsCalibrationSetupXmlReader::readXml() {

        try {
            std::string& mySubjectFile(*(ceinmsCalibrationSetupPointer_->subjectFile().begin()));
            std::string& myCalibrationExecutionFile(*(ceinmsCalibrationSetupPointer_->calibrationFile().begin()));
            std::string& myEmgGeneratorFile(*(ceinmsCalibrationSetupPointer_->excitationGeneratorFile().begin()));
            std::string& myOutputSubjectFile(*(ceinmsCalibrationSetupPointer_->outputSubjectFile().begin()));

        }
        catch (const xml_schema::exception& e) {
            cout << e << endl;
            exit(EXIT_FAILURE);
        }
    }

    std::string CeinmsCalibrationSetupXmlReader::getSubjectFile()
    {
        std::string fileName(*(ceinmsCalibrationSetupPointer_->subjectFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    std::string CeinmsCalibrationSetupXmlReader::getCalibrationFile()
    {
        std::string fileName(*(ceinmsCalibrationSetupPointer_->calibrationFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    std::string CeinmsCalibrationSetupXmlReader::getExcitationGeneratorFile()
    {
        std::string fileName(*(ceinmsCalibrationSetupPointer_->excitationGeneratorFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    std::string CeinmsCalibrationSetupXmlReader::getOutputSubjectFile()
    {
        std::string fileName(*(ceinmsCalibrationSetupPointer_->outputSubjectFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    bool CeinmsCalibrationSetupXmlReader::writeTemplateCeinmsCalibrationSetupFile(const std::string& templateFile)
    {
        std::ofstream templateFileStream(templateFile);
        if (!templateFileStream.is_open())
            return false;
        CeinmsCalibrationType templateCeinmsSetup;
        templateCeinmsSetup.subjectFile().push_back("subjectUncalibrated.xml");
        templateCeinmsSetup.calibrationFile().push_back("calibration.xml");
        templateCeinmsSetup.excitationGeneratorFile().push_back("excitationGenerator.xml");
        templateCeinmsSetup.outputSubjectFile().push_back("subjectCalibrated.xml");
        ceinmsCalibration(templateFileStream, templateCeinmsSetup);
        templateFileStream.close();
        return true;
    }
}
