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

#ifndef ceinms_QueuesToStorageFiles_h
#define ceinms_QueuesToStorageFiles_h

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "OutputConnectors.h"
#include "ceinms/InputConnectors.h"

namespace ceinms {
    class QueuesToStorageFiles {
    public:
        QueuesToStorageFiles() = delete;
        QueuesToStorageFiles(const QueuesToStorageFiles&) = delete;
        QueuesToStorageFiles& operator=(const QueuesToStorageFiles&) = delete;

        template <typename NMSmodelT>
        QueuesToStorageFiles(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, const NMSmodelT& subject, const std::vector< std::string >& valuesToWrite, const std::string& outputDir = "./Output/");
        ~QueuesToStorageFiles() = default;
        void operator()();

    private:
        void initFile(const std::string& valueName, const std::vector< OutputConnectors::FrameType >& dataToWrite);
        void addLog(std::string valueName);

        std::string outputDir_;
        std::string separator_;
        std::map< std::string, std::shared_ptr<std::ostream> > outFiles_;
        std::vector<std::string> valuesToWrite_;
        std::vector<std::string> torqueNames_;
        std::vector<std::string> muscleNames_;
        InputConnectors& inputConnectors_;
        OutputConnectors& outputConnectors_;

    };


    template <typename NMSmodelT>
    QueuesToStorageFiles::QueuesToStorageFiles(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, const NMSmodelT& subject, const std::vector< std::string >& valuesToWrite, const std::string& outputDir)
        :outputDir_(outputDir), separator_("\t"), valuesToWrite_(valuesToWrite), inputConnectors_(inputConnectors), outputConnectors_(outputConnectors) {
        outputDir_ += "/";
        boost::filesystem::path dir(outputDir_);
        if (!boost::filesystem::exists(dir)) {
            if (!boost::filesystem::create_directory(dir)) {
                std::cout << "Error: Cannot create the output directory " + outputDir_ << std::endl;
                exit(EXIT_FAILURE);
            }
            std::cout << "Created output directory " + dir.string() << std::endl;
        }
        else std::cout << "Using " + dir.string() + " as output directory\n";

        subject.getDoFNames(torqueNames_);
        subject.getMuscleNames(muscleNames_);

    }
}
#endif
