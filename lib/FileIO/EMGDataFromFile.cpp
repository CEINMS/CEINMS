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

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <sstream>
using std::stringstream;
#include <stdlib.h>

#include "EMGDataFromFile.h"

namespace ceinms {
    template <typename EMGgenerator>
    EMGDataFromFile<EMGgenerator>::EMGDataFromFile(const string& EMGDataFilename, const std::string& emgGeneratorFile)
        :EMGDataFile_(EMGDataFilename.c_str()), EMGgenerator_(emgGeneratorFile) {

        if (!EMGDataFile_.is_open()) {
            cout << "ERROR emg file: " << EMGDataFilename << " could not be open\n";
            exit(EXIT_FAILURE);
        }

        string line;
        getline(EMGDataFile_, line, '\n');
        while (line != "endheader" && line != "endheader\r" && !EMGDataFile_.eof())
        {
            if (size_t foundInfo = line.find("nRows=") != string::npos)
            {
                noTimeSteps_ = atoi(line.substr(6).c_str());
            }
            if (size_t foundInfo = line.find("datarows ") != string::npos)
            {
                noTimeSteps_ = atoi(line.substr(9).c_str());
            }
            if (size_t foundInfo = line.find("nColumns=") != string::npos)
            {
                noMuscles_ = atoi(line.substr(9).c_str()) - 1; //nColumns includes the "time" column
            }
            if (size_t foundInfo = line.find("datacolumns ") != string::npos)
            {
                noMuscles_ = atoi(line.substr(12).c_str()) - 1; //nColumns includes the "time" column
            }
            getline((EMGDataFile_), line, '\n');
        }
        // reading muscles
        getline((EMGDataFile_), line, '\n');
        stringstream myStream(line);
        string nextColumnName;
        // the first is the "Time"
        string timeName;
        myStream >> timeName;
        // then we have their names
        int noReadColumns = 0;
        do {
            myStream >> nextColumnName;
            muscleNames_.push_back(nextColumnName);
        } while (!myStream.eof() && ++noReadColumns < noMuscles_);


        if (noMuscles_ != muscleNames_.size()) {
            cout << "Something is wrong. " << noMuscles_ << " muscles should be in the file "
                << "and we have : " << muscleNames_.size();
            exit(EXIT_FAILURE);
        }

        if (!EMGgenerator_.checkInputSignalsNames(muscleNames_)) {
            vector<string> musclesNamesFromGenerator;
            EMGgenerator_.getInputSignalsNames(musclesNamesFromGenerator);
            std::cout << " generator names - file names\n";
            for (unsigned i = 0; i < musclesNamesFromGenerator.size() && i < muscleNames_.size(); ++i) {
                string genName(musclesNamesFromGenerator.at(i)), fileName(muscleNames_.at(i)), separator("  -  ");
                if (genName != fileName) separator = "  <>  ";
                std::cout << genName << separator << fileName << std::endl;
            }
            cout << "THE EMG generator is not able to generate EMG for your muscles starting from muscle in the file! (muscle names are different)\n";

            exit(EXIT_FAILURE);
        }

        //TODO fix resize
        currentReadEMG_.resize(EMGgenerator_.getNoInputSignals());
        currentEMGData_.resize(EMGgenerator_.getNoExcitations());
        currentDataTime_ = 0.;
        currentTimeStep_ = 0;
    }

    template <typename EMGgenerator>
    void EMGDataFromFile<EMGgenerator>::readNextEmgData()  {

        // first we build a vector of emg read from the file

        string line;
        getline(EMGDataFile_, line, '\n');
        stringstream myStream(line);
        double value;
        currentReadEMG_.clear();
        myStream >> currentDataTime_;
        // cout << "EMGdatafromfile in emg.txt: time step "<< currentDataTime_ << endl;
        int noReadMuscles = 0;
        do {
            myStream >> value;
            //  cout << "EMGdatafromfile in emg.txt: value "<< value << endl;
            currentReadEMG_.push_back(value);
        } while (!myStream.eof() && ++noReadMuscles < noMuscles_);
        // cout << "EMGDataFromFile: just read " << currentReadEMG_.size() << " values\n";
        EMGgenerator_.convert(currentReadEMG_, currentEMGData_);

        /*
          int noFromMuscles = EMGgenerator_.getNoFromMuscles();
          // 1. first their names
          for (int i = 0; i < noFromMuscles; ++i) {
          double value;
          EMGDataFile_ >> value;
          currentReadEMG_[i] = value;
          }

          // then we convert them on what they need
          EMGgenerator_.convert(currentReadEMG_, currentEMGData_);
          */
        ++currentTimeStep_;

    }

    template <typename EMGgenerator>
    EMGDataFromFile<EMGgenerator>::~EMGDataFromFile() {
        EMGDataFile_.close();
    }
}

#include "policyTemplates.h"
