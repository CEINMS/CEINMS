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

#ifndef ceinms_EmgFromFile_h
#define ceinms_EmgFromFile_h

#include <string>
#include <vector>
#include <iostream>

#include "ceinms/EmgFromX.h"
#include "ceinms/EMGDataFromFile.h"
#include "ceinms/ExcitationGeneratorFromXml.h"
#include "ceinms/Mapper.h"

#define LOG_EMGFromFile

namespace ceinms {
    class EmgFromFile :public EmgFromX
    {
    public:
        template <typename NMSModelT>
        EmgFromFile(
            EmgQueue& outputEmgQueue,
            rtb::Concurrency::Latch& doneWithSubscriptions,
            rtb::Concurrency::Latch& doneWithExecution,
            const NMSModelT& subject,
            const std::string& emgFilename,
            const std::string& emgGeneratorFilename);
        void operator()();
        virtual ~EmgFromFile();
    private:
        Mapper excitationMapper_;
        std::string inputDir_;
        EMGDataFromFile<ExcitationGeneratorFromXml> emgData_;
    };

    template <typename NMSModelT>
    EmgFromFile::EmgFromFile(
        EmgQueue& outputEmgQueue,
        rtb::Concurrency::Latch& doneWithSubscriptions,
        rtb::Concurrency::Latch& doneWithExecution,
        const NMSModelT& subject,
        const std::string& emgFilename,
        const std::string& emgGeneratorFilename) :
        EmgFromX(outputEmgQueue, doneWithSubscriptions, doneWithExecution, subject), emgData_(emgFilename, emgGeneratorFilename)
    {

#ifdef LOG_EMGFromFile
        std::cout << "\nEMG: Reading emg file..." << emgFilename << std::endl;
#endif

        std::vector<std::string> emgMusclesNames;
        emgData_.getMusclesNames(emgMusclesNames);
        excitationMapper_.setNames(emgMusclesNames, musclesNames_);
        auto mappedExcitationNames(excitationMapper_.map(emgMusclesNames, std::string("None")));
        std::cout << "Muscle excitations to muscle mapping:\n";
        for (int i(0); i < mappedExcitationNames.size(); ++i) {
            std::cout << mappedExcitationNames.at(i) << " -> " << musclesNames_.at(i);
            if (mappedExcitationNames.at(i) != musclesNames_.at(i))
                std::cout << " WARNING. Mapping may be incorrect!";
            std::cout << std::endl;

        }

    }
}

#endif
