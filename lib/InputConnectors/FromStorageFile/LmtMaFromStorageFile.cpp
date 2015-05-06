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

#include "LmtMaFromStorageFile.h"
#include "DataFromStorageFile.h"
#include "InputConnectors.h"
#include "Utilities.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>

#define LOG

namespace ceinms {
    void LmtMaFromStorageFile::operator()()
    {
        // 1. wait - if required - for the required subscriptions to happen
        inputConnectors_.doneWithSubscription.wait();

        double time;
        while (lmtData_.areStillData())
        {
            lmtData_.readNextData();
            auto newLmtData = lmtData_.getCurrentData();
            time = lmtData_.getCurrentTime(); //lmt and momentArms times are equal, ALWAYS!

            vector<double> selectedLmtData(musclesNames_.size());
            for (int i = 0; i < musclesNames_.size(); ++i)
                selectedLmtData.at(i) = newLmtData.at(musclePositionInLmtStorage_.at(i));

            updateLmt(selectedLmtData, time);

            for (unsigned int currentDof = 0; currentDof < dofNames_.size(); ++currentDof)
            {

                (*maDataStorageFiles_.at(currentDof)).readNextData();
                auto newMaData = (*maDataStorageFiles_.at(currentDof)).getCurrentData();

                vector<double> selectedMaData(musclePositionsInMaStorages_.at(currentDof).size());
                for (int i = 0; i < selectedMaData.size(); ++i)
                    selectedMaData.at(i) = newMaData.at(musclePositionsInMaStorages_.at(currentDof).at(i));

                updateMomentArms(selectedMaData, time, currentDof);
            }
        }

        vector<double> endOfData;
        updateLmt(endOfData, std::numeric_limits<double>::infinity());
        for (unsigned int i = 0; i < dofNames_.size(); ++i)
            updateMomentArms(endOfData, std::numeric_limits<double>::infinity(), i);

        //SyncTools::Shared::lmtProducingDone.notify(); //used for validate curve only


#ifdef LOG
        cout << "\nLmtMa: lmtMa DONE\n";
#endif

    }
}
