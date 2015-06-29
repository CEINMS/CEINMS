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

#ifndef ceinms_QueueToStorageFile_h
#define ceinms_QueueToStorageFile_h

#include "ceinms/concurrency/Latch.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/QueueData.h"
#include "boost/filesystem.hpp"
#include <vector>
#include <memory>
#include <ostream>

namespace ceinms {

    template<typename DataType>
    class QueueToStorageFile {
    public:
        using FrameType = QueueData < DataType > ;
        using QueueType = Concurrency::Queue < FrameType > ;
        QueueToStorageFile() = delete;
        QueueToStorageFile(const QueueToStorageFile&) = delete;
        QueueToStorageFile& operator=(const QueueToStorageFile&) = delete;

        QueueToStorageFile(
            QueueType& inputQueue,
            Concurrency::Latch& subscriptionLatch,
            Concurrency::Latch& readyToWriteLatch,
            const std::vector<std::string>& columnLabels,
            const std::string& filename,
            const std::string& outputDir = "./Output/",
            const std::string& extension = "sto");
        ~QueueToStorageFile() = default;
        void operator()();

    private:
        void initFile();
        void writeToFile();
        QueueType& inputQueue_;
        Concurrency::Latch& subscriptionLatch_;
        Concurrency::Latch& readyToWriteLatch_;
        std::vector<std::string> columnLabels_;
        std::string outputDir_;
        std::string separator_;
        std::string valueName_;
        std::string extension_;
        std::shared_ptr<std::ofstream> outFile_;
        std::vector< FrameType > dataToWrite_;
    };
}

#include "QueueToStorageFile.cpp"
#endif