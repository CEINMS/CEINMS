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

#include "ceinms/InputDataXmlReader.h"
#include <string>
using std::string;
#include <map>
using std::map;
#include <iostream>
using std::cout;
using std::endl;
#include <limits>
#include "inputData-schema.hxx"

#include "validation.h"
#include "ceinms/FileUtils.h"
using InputDataXsd::InputDataType;

namespace ceinms {
    InputDataXmlReader::InputDataXmlReader(const string& filename)
    {

        try {
            std::unique_ptr<InputDataType> inputDataPointer(parseAndValidate<InputDataType>(filename, inputData_schema, sizeof(inputData_schema)));
            inputDataPointer_ = std::move(inputDataPointer);
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

    void InputDataXmlReader::readXml() {

        try {
            std::string& myLmtFile(*(inputDataPointer_->muscleTendonLengthFile().begin()));
            if (inputDataPointer_->excitationsFile().size() > 0)
                std::string& myEmgFile(*(inputDataPointer_->excitationsFile().begin()));
            //TODO std::string& myMomentArmDir(*(inputDataPointer_->momentArmsFDirectory().begin()));
            if (inputDataPointer_->externalTorquesFile().size() > 0)
                std::string& myExtTorqueFile(*(inputDataPointer_->externalTorquesFile().begin()));
        }
        catch (const xml_schema::exception& e) {
            cout << e << endl;
            exit(EXIT_FAILURE);
        }
    }

    std::string InputDataXmlReader::getLmtFile()
    {
        std::string fileName(*(inputDataPointer_->muscleTendonLengthFile().begin()));
        FileUtils::trim(fileName);
        if (FileUtils::isAbsolute(fileName.c_str()))
            return fileName;
        else
            return FileUtils::simplify(filepath_ + fileName);
    }

    std::map<std::string, std::string> InputDataXmlReader::getMaFiles()
    {
        map<string, string> result;
        for (auto it : inputDataPointer_->momentArmsFiles().begin()->momentArmsFile())
        {
            std::string pathToMaFile = it.c_str();
            FileUtils::trim(pathToMaFile);
            if (!FileUtils::isAbsolute(pathToMaFile.c_str()))
                pathToMaFile = FileUtils::simplify(filepath_ + pathToMaFile);
            result.insert(std::pair<string, string>(it.dofName(), pathToMaFile));
        }
        // return filepath_ + *(inputDataPointer_->momentArmsDirectory().begin());
        return result;
    }

    std::string InputDataXmlReader::getExcitationsFile(bool relative)
    {
        if (inputDataPointer_->excitationsFile().size() < 1)
            return "";
        std::string fileName(*(inputDataPointer_->excitationsFile().begin()));
        FileUtils::trim(fileName);
        if (!FileUtils::isAbsolute(fileName.c_str()))
            if (relative)
                fileName = FileUtils::simplify(fileName);
            else
                fileName = FileUtils::makeGeneric(filepath_ + fileName);
        return fileName;
    }

    std::string InputDataXmlReader::getExternalTorqueFile(bool relative)
    {
        if (inputDataPointer_->externalTorquesFile().size() < 1)
            return "";
        std::string fileName(*(inputDataPointer_->externalTorquesFile().begin()));
        FileUtils::trim(fileName);
        if (!FileUtils::isAbsolute(fileName.c_str()))
            if (relative)
                fileName = FileUtils::simplify(fileName);
            else
                fileName = FileUtils::makeGeneric(filepath_ + fileName);
        return fileName;
    }

    std::string InputDataXmlReader::getMotionFile(bool relative) const {
    

        try {
            if (inputDataPointer_->motionFile().size() < 1)
                return "";
            std::string fileName(*(inputDataPointer_->motionFile().begin()));
            FileUtils::trim(fileName);
            if (!FileUtils::isAbsolute(fileName.c_str()))
                if (relative)
                    fileName = FileUtils::simplify(fileName);
                else
                    fileName = FileUtils::makeGeneric(filepath_ + fileName);
            return fileName;
        }
        catch (...)
        {
            std::cout << "No OpenSim motion file associated to the trial\n";
            return "";
        }
    }


    std::string InputDataXmlReader::getExternalLoadsFile(bool relative) const {
    
        try {
            if (inputDataPointer_->externalLoadsFile().size() < 1)
                return "";
            std::string fileName(*(inputDataPointer_->externalLoadsFile().begin()));
            FileUtils::trim(fileName);
            if (!FileUtils::isAbsolute(fileName.c_str()))
                if (relative)
                    fileName = FileUtils::simplify(fileName);
                else
                    fileName = FileUtils::makeGeneric(filepath_ + fileName);
            return fileName;
        }
        catch (...)
        {
            std::cout << "No OpenSim external load file associated to the trial\n";
            return "";
        }
    }

    double  InputDataXmlReader::getStartTime() const {

        if (inputDataPointer_->startStopTime().size() > 0) 
            return inputDataPointer_->startStopTime().begin()->at(0);
        return std::numeric_limits<double>::lowest();
    }

    double  InputDataXmlReader::getStopTime() const {

        if (inputDataPointer_->startStopTime().size() > 0)
            return inputDataPointer_->startStopTime().begin()->at(1);
        return std::numeric_limits<double>::max();
    }
}
