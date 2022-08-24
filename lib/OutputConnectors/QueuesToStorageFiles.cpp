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

#include "ceinms/QueuesToStorageFiles.h"

namespace ceinms {

    void QueuesToStorageFiles::addLog(std::string valueName) {
        std::string filename = valueName + ".sto";
        std::string outFilename = outputDir_ + filename;

        std::shared_ptr<std::ofstream> file_ptr(new std::ofstream(outFilename.c_str()));
        if (!(file_ptr->is_open()))  {
            std::cout << "ERROR: " + filename + " cannot be opened!\n";
            exit(EXIT_FAILURE);
        }

        outFiles_.insert(std::make_pair(valueName, file_ptr));
    }

    void QueuesToStorageFiles::operator()() {
        for (auto& it : valuesToWrite_) {
            auto q = inputLogQueue_.find(it);
            if (q == inputLogQueue_.end()) {
                std::cout << it << " queue was not found\n";
                exit(EXIT_FAILURE);
            }
            else {
                q->second->subscribe();
                addLog(it);
            }
        }

        doneWithSubscriptions_.wait();

        readyToWrite_.wait();

        for (auto& currentValue : valuesToWrite_) {

            auto currentQueue = inputLogQueue_.find(currentValue);
            if (currentQueue == inputLogQueue_.end()) {
                std::cout << currentValue << " queue was not found\n";
                exit(EXIT_FAILURE);
            }

            std::vector<ScalarVectorFrame> dataToWrite;
            while (auto currentFrame{currentQueue->second->pop()}) {
                dataToWrite.push_back(currentFrame.value());
            }

            initFile(currentValue, dataToWrite);
            auto q = outFiles_.find(currentValue);
            auto outFile = (q->second);
            for (auto& dataIt : dataToWrite) {
                *outFile << dataIt.time << separator_;
                for (auto& sampleIt : dataIt.data)
                    *outFile << sampleIt << separator_;
                *outFile << std::endl;
            }
        }
    }

    void QueuesToStorageFiles::initFile(const std::string& valueName, const std::vector<ScalarVectorFrame>& dataToWrite) {
        if (dataToWrite.size() < 1) {
            std::cout << "No " << valueName << "data to write!" << std::endl;
            return;
        }
        auto q = outFiles_.find(valueName);
        if (q == outFiles_.end()) {
            std::cout << valueName << " file was not found\n";
            exit(EXIT_FAILURE);
        }

        auto outFile = (q->second);
        *outFile << "CEINMS output" << std::endl;
        *outFile << "datacolumns " << dataToWrite.back().data.size() + 1 << std::endl;
        *outFile << "datarows " << dataToWrite.size() << std::endl;
        *outFile << "endheader" << std::endl;
        *outFile << "time" + separator_;
        if (valueName == "Torques")
            for (auto& it : torqueNames_)
                *outFile << it << separator_;
        else if (valueName == "MusclesContribution")
            for (auto& it : torqueNames_)
                for (auto& jt : muscleNamesOnDofs_.at(it))
                    *outFile << jt + "_" + it << separator_;
        else
            for (auto& it : muscleNames_)
                *outFile << it << separator_;
        *outFile << std::endl;
    }
}