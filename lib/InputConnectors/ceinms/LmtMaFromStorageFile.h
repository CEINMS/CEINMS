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

#ifndef ceinms_LmtMaFromStorageFile_h
#define ceinms_LmtMaFromStorageFile_h

#include <string>
#include <vector>
#include <memory>
#include <limits>

#include "LmtMaFromX.h"
#include "Utilities.h"
#include "ceinms/DataFromStorageFile.h"

namespace ceinms {

    class LmtMaFromStorageFile :public LmtMaFromX {
    public:
        template<typename NMSModelT>
        LmtMaFromStorageFile(
            LmtQueue& outputLmtQueue,
            MultipleMomentArmsQueues& outputMomentArmsQueue,
            rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
            const NMSModelT& subject,
            const std::string& lmtDataFilename,
            const std::vector< std::string>& maDataFileName);
        void operator()();
		void setStartStopTimes(double start, double stop) {
			startTime_ = start;
			stopTime_ = stop;
		}

    private:
        std::string dataDirectory_;
        std::vector<size_t> musclePositionInLmtStorage_;
        std::vector<std::vector<size_t>> musclePositionsInMaStorages_;
        DataFromStorageFile lmtData_;
        std::vector<std::unique_ptr<DataFromStorageFile>> maDataStorageFiles_;
		double startTime_;
		double stopTime_;
    };

    template<typename NMSModelT>
    LmtMaFromStorageFile::LmtMaFromStorageFile(
        LmtQueue& outputLmtQueue,
        MultipleMomentArmsQueues& outputMomentArmsQueue,
		rtb::Concurrency::Latch& doneWithSubscriptions,
		rtb::Concurrency::Latch& doneWithExecution,
        const NMSModelT& subject,
        const std::string& lmtDataFilename,
        const std::vector< std::string>& maDataFileName) :
        LmtMaFromX(outputLmtQueue, outputMomentArmsQueue, doneWithSubscriptions, doneWithExecution, subject),
        lmtData_(lmtDataFilename),
		startTime_(std::numeric_limits<double>::lowest()),
		stopTime_(std::numeric_limits<double>::max()) {

        // 1. Open the input files
        for (std::string it : maDataFileName)
            maDataStorageFiles_.emplace_back(new DataFromStorageFile(it));

        // 2. setup the conversion matrixes
        std::vector<std::string> musclesNamesFromStorageFile = lmtData_.getColumnNames();
        musclePositionInLmtStorage_ = findMapping(musclesNames_, musclesNamesFromStorageFile);

        musclePositionsInMaStorages_.resize(dofNames_.size());
        size_t currentDof = 0;
        for (auto& it : maDataStorageFiles_)
        {
            std::vector<std::string> muscleNamesFromStorageFile = it->getColumnNames();
            musclePositionsInMaStorages_.at(currentDof) = findMapping(muscleNamesOnDofs_.at(currentDof), muscleNamesFromStorageFile);
            currentDof++;
        }
    }
}

#endif
