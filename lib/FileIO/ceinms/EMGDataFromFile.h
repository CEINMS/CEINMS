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

#ifndef ceinms_EMGDataFromFile_h
#define ceinms_EMGDataFromFile_h

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

namespace ceinms {
    template <typename EMGgenerator>
    class EMGDataFromFile {
    public:
        EMGDataFromFile() = delete;// { std::cout << "You should not be there\n"; }
        EMGDataFromFile(const std::string& inputFile, const std::string& emgGeneratorFile);
        const std::vector<std::string>& getMusclesNames() const { return EMGgenerator_.getExcitationsNames(); }
        void getMusclesNames(std::vector< std::string >& muscleNames) const { muscleNames = EMGgenerator_.getExcitationsNames(); }
        void readNextEmgData();
        inline double getCurrentTime() const { return currentDataTime_; }
        bool areStillData() const { return currentTimeStep_ < noTimeSteps_; }
        const std::vector<double>& getCurrentData() const { return currentEMGData_; }
        int getNoTimeSteps() const { return noTimeSteps_; }
        ~EMGDataFromFile();
    private:
        std::ifstream EMGDataFile_;
        int noMuscles_;
        int noTimeSteps_;
        int currentTimeStep_;
        double currentDataTime_;
        std::vector<std::string> muscleNames_;
        std::vector<double> currentReadEMG_;
        std::vector<double> currentEMGData_;
        EMGgenerator EMGgenerator_;
    };
}

#endif
