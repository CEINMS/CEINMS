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
 * Author(s): C. Pizzolato, M. Reggiani, E. Ceseracciu                        *
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

#include "ceinms/LoggerOnQueues.h"
#include "ceinms/OutputConnectors.h"
#include <vector>
#include <string>
#include <iostream>

namespace ceinms{
    LoggerOnQueues::LoggerOnQueues(OutputConnectors& outputConnector, const std::vector< std::string >& valuesToLog) :
        outputConnectors_(outputConnector) {

        for (auto& it : valuesToLog)
            outputConnectors_.logQueues.insert(std::make_pair(it, new rtb::Concurrency::Queue< OutputConnectors::FrameType>));
    }


    void LoggerOnQueues::log(double time, const OutputConnectors::DataType& dataToLog, const std::string& valueToLog) {

        OutputConnectors::FrameType dataToPush;
        dataToPush.data = dataToLog;
        dataToPush.time = time;

        auto currentQueue = outputConnectors_.logQueues.find(valueToLog);

        if (currentQueue == outputConnectors_.logQueues.end()) {
            std::cout << "queue for " << valueToLog << " was not found\n";
            exit(EXIT_FAILURE);
        }
        else
            currentQueue->second->push(dataToPush);
    }
}
