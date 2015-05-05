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
#include <fstream>

#include "CeinmsSetupXmlReader.h"
#include "validation.h"
#include "ceinmsSetup-schema.hxx"
using CeinmsSetupXsd::CeinmsType;
#include "FileUtils.h"

namespace ceinms {
    CeinmsSetupXmlReader::CeinmsSetupXmlReader(const string& filename)
    {

        try {
            std::auto_ptr<CeinmsType> ceinmsDataPointer(parseAndValidate<CeinmsType>(filename, ceinmsSetup_schema, sizeof(ceinmsSetup_schema)));
            ceinmsSetupPointer_ = ceinmsDataPointer;
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

    void CeinmsSetupXmlReader::readXml() {

        try {
            std::string& mySubjectFile(*(ceinmsSetupPointer_->subjectFile().begin()));
            std::string& myInputDataFile(*(ceinmsSetupPointer_->inputDataFile().begin()));
            std::string& myExecutionFile(*(ceinmsSetupPointer_->executionFile().begin()));
            std::string& myEmgGeneratorFile(*(ceinmsSetupPointer_->excitationGeneratorFile().begin()));
            std::string& myOutputDirectory(*(ceinmsSetupPointer_->outputDirectory().begin()));

        }
        catch (const xml_schema::exception& e) {
            cout << e << endl;
            exit(EXIT_FAILURE);
        }
    }

    std::string CeinmsSetupXmlReader::getSubjectFile()
    {
        std::string fileName(*(ceinmsSetupPointer_->subjectFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    std::string CeinmsSetupXmlReader::getInputDataFile()
    {
        std::string fileName(*(ceinmsSetupPointer_->inputDataFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    std::string CeinmsSetupXmlReader::getExecutionFile()
    {
        std::string fileName(*(ceinmsSetupPointer_->executionFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    std::string CeinmsSetupXmlReader::getExcitationGeneratorFile()
    {
        std::string fileName(*(ceinmsSetupPointer_->excitationGeneratorFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    std::string CeinmsSetupXmlReader::getOutputDirectory()
    {
        std::string fileName(*(ceinmsSetupPointer_->outputDirectory().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
    }

    bool CeinmsSetupXmlReader::writeTemplateCeinmsSetupFile(const string& templateFile)
    {
        std::ofstream templateFileStream(templateFile);
        if (!templateFileStream.is_open())
            return false;
        CeinmsType templateCeinmsSetup;
        templateCeinmsSetup.subjectFile().push_back("subject.xml");
        templateCeinmsSetup.inputDataFile().push_back("inputData.xml");
        templateCeinmsSetup.executionFile().push_back("execution.xml");
        templateCeinmsSetup.excitationGeneratorFile().push_back("excitationGenerator.xml");
        templateCeinmsSetup.outputDirectory().push_back("./Output");
        CeinmsSetupXsd::ceinms(templateFileStream, templateCeinmsSetup);
        templateFileStream.close();
        return true;
    }
}
