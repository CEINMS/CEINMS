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

#ifndef ceinms_DataFromStorageFile_h
#define ceinms_DataFromStorageFile_h

#include <fstream>
#include <iostream>
#include <vector>
#include <memory>

/**
 * \brief This class read from a file either lmt or ma data
 * You should have a file that includes
 * write here the structure of the file
 */
namespace ceinms {
    class DataFromStorageFile {
    public:
        DataFromStorageFile() = delete;
        DataFromStorageFile(const std::string& inputFile);
        const std::vector<std::string>& getColumnNames() const { return columnNames_; }
        void readNextData();
        int getNoTimeSteps() const { return noTimeSteps_; }
        bool areStillData() const { return currentTimeStep_ < noTimeSteps_; }
        inline double getCurrentTime() const { return currentDataTime_; }
        const std::vector<double>& getCurrentData() const;
        ~DataFromStorageFile();
        DataFromStorageFile(const DataFromStorageFile& orig) = delete;
        DataFromStorageFile& operator=(const DataFromStorageFile& orig) = delete;
    private:
        std::string dataFileName_;
        std::ifstream dataFile_;
        unsigned int noColumns_;
        std::vector<std::string> columnNames_;
        int currentTimeStep_;
        int noTimeSteps_;
        double currentDataTime_;
        std::vector<double> currentData_;
    };
}
#endif
