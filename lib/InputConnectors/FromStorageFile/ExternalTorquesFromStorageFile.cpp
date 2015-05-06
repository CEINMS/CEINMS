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
 * Author(s): E. Ceseracciu, M. Reggiani                                      *
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

#include "ExternalTorquesFromStorageFile.h"
#include "ExternalTorquesFromX.h"
#include "DataFromStorageFile.h"
#include "InputConnectors.h"


#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>
#include "Utilities.h"

#define LOG

namespace ceinms  {
    ExternalTorquesFromStorageFile::~ExternalTorquesFromStorageFile()
    {
        if (externalTorquesDataFromFile_ != nullptr)
            delete externalTorquesDataFromFile_;
    }

    bool ExternalTorquesFromStorageFile::externalTorquesFileExist(const string& fileName)
    {
        std::ifstream dataFile(fileName.c_str());
        bool exist = dataFile.is_open();
        dataFile.close();
        return exist;
    }

    void ExternalTorquesFromStorageFile::operator()()
    {

        inputConnectors_.doneWithSubscription.wait();

        if (inputConnectors_.externalTorquesAvailable)
        {
            while (externalTorquesDataFromFile_->areStillData())
            {
                externalTorquesDataFromFile_->readNextData();
                auto newTorquesData = externalTorquesDataFromFile_->getCurrentData();
                vector<double> selectedTorquesData(dofNames_.size());
                for (int i = 0; i < dofNames_.size(); ++i) {
                    selectedTorquesData.at(i) = newTorquesData.at(dofPosInStorage_.at(i));
                }
                updateExternalTorques(selectedTorquesData, externalTorquesDataFromFile_->getCurrentTime());
            }

            vector<double> endOfTorques;
            updateExternalTorques(endOfTorques, std::numeric_limits<double>::infinity());

        }
        else {
#ifdef LOG
            cout << "\n ExtTorque: external Torques not available " << endl;
#endif
        }

#ifdef LOG
        cout << "\nexternalTorque DONE\n";
#endif

    }
}
