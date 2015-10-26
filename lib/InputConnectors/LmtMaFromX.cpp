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

#include "ceinms/LmtMaFromX.h"
#include "ceinms/EndOfData.h"
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>
#include <limits>

namespace ceinms {

    void LmtMaFromX::updateLmt(const LmtData& currentLmtData, double currentTime) {

        LmtFrame lmtFrameToPush;
        lmtFrameToPush.data = currentLmtData;
        lmtFrameToPush.time = currentTime;
        outputLmtQueue_.push(lmtFrameToPush);
    }

    void LmtMaFromX::updateMomentArms(const MomentArmsData& currentMomentArmsData, double currentTime, unsigned int whichDof) {

        MomentArmsFrame momentArmsFrameToPush;
        momentArmsFrameToPush.data = currentMomentArmsData;
        momentArmsFrameToPush.time = currentTime;
        outputMomentArmsQueue_.at(whichDof)->push(momentArmsFrameToPush);
    }

    void LmtMaFromX::doneWithSubscriptions() {

        doneWithSubscriptions_.wait();
    }

    void LmtMaFromX::doneWithExecution() {

        doneWithExecution_.wait();
    }

    void LmtMaFromX::sendLmtEndOfData() {

        outputLmtQueue_.push(EndOfData::get<LmtFrame>());
    }

    void LmtMaFromX::sendMaEndOfData() {

        for (unsigned i(0); i < dofNames_.size(); ++i)
            outputMomentArmsQueue_.at(i)->push(EndOfData::get<MomentArmsFrame>());
    }

    void LmtMaFromX::initialiseMomentArmsQueue() {

        outputMomentArmsQueue_.clear();
        for (size_t i(0); i < dofNames_.size(); ++i)
            outputMomentArmsQueue_.emplace_back(new MomentArmsQueue());
    }

    LmtMaFromX::~LmtMaFromX() { }
}