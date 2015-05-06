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
 * Author(s): C. Pizzolato, M. Reggiani                                       *
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

#ifndef ceinms_EMGFromFile_h
#define ceinms_EMGFromFile_h

#include <string>
#include <vector>
#include <iostream>

#include "EMGFromX.h"
#include "EMGDataFromFile.h"
#include "ExcitationGeneratorFromXml.h"
#include "InputConnectors.h"

#define LOG

namespace ceinms {
    class EMGFromFile :public EMGFromX
    {
    public:
        template <typename NMSModelT>
        EMGFromFile(InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& emgFilename, const std::string& emgGeneratorFilename);
        void operator()();
        virtual ~EMGFromFile();
    private:
        std::string inputDir_;
        EMGDataFromFile<ExcitationGeneratorFromXml> emgData_;
    };


    template <typename NMSModelT>
    EMGFromFile::EMGFromFile(InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& emgFilename, const std::string& emgGeneratorFilename)
        :EMGFromX(inputConnectors, subject), emgData_(emgFilename, emgGeneratorFilename)
    {

#ifdef LOG
        std::cout << "\nEMG: Reading emg file..." << emgFilename << std::endl;
#endif

        std::vector<std::string> emgMusclesNames;
        emgData_.getMusclesNames(emgMusclesNames);
        if (!(musclesNames_ == emgMusclesNames)) {
            std::cout << "\n\n ERROR: Muscle names from emg file and subject differ!\n";
            std::cout << "Subject muscles: ";
            for (auto it : musclesNames_)
                std::cout << it << "\t";
            std::cout << "\nEMG muscles: ";
            for (auto it : emgMusclesNames)
                std::cout << it << "\t";
            std::cout << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

#endif
