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

#include <memory>
using std::shared_ptr;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <algorithm>
#include <chrono>

#include "ceinms/LmtMaFromMcbs.h"
#include "ceinms/EndOfData.h"
#include <ceinms/ArrayConverter.h>

namespace ceinms {
   
    void LmtMaFromMcbs::readDefaultMomentArmsFromOsim(
        const string& osimModelFIlename) {

        OpenSim::Model model(osimModelFIlename);
		SimTK::State s(model.initSystem());
        OpenSim::Array<string> osimMuscleNames_;
        ArrayConverter::fromStdVector(osimMuscleNames_, musclesNames_);

        std::unique_ptr<OpenSim::MuscleAnalysis> muscleTendonLengthAnalysis(new OpenSim::MuscleAnalysis(&model));
        muscleTendonLengthAnalysis->setMuscles(osimMuscleNames_);
        muscleTendonLengthAnalysis->setComputeMoments(true);
        muscleTendonLengthAnalysis->setPrintResultFiles(false);
        model.addAnalysis(muscleTendonLengthAnalysis.get());
        s = model.initSystem();
        model.getMultibodySystem().realize(s, SimTK::Stage::Velocity);
        muscleTendonLengthAnalysis->step(s, 1);
        const OpenSim::ArrayPtrs<OpenSim::MuscleAnalysis::StorageCoordinatePair>& momentArmStorage(muscleTendonLengthAnalysis->getMomentArmStorageArray());
        size_t numCoordinates = momentArmStorage.getSize();

        for (size_t i(0); i < numCoordinates; ++i) {
            string currentCoordinateName(momentArmStorage[i]->q->getName());
            if (find(dofsNamesFromOsimModel_.begin(), dofsNamesFromOsimModel_.end(), currentCoordinateName) != dofsNamesFromOsimModel_.end()) {
                vector<double> momentArmsOnDofs;
                for (auto muscleName : muscleNamesOnDofsMap_.at(currentCoordinateName)){
                    OpenSim::Array<double> col;
                    momentArmStorage[i]->momentArmStore->getDataColumn(muscleName, col);
                    momentArmsOnDofs.push_back(col.get(0));
                }
                defaultMomentArmsOnDofsMap_.insert(std::make_pair(currentCoordinateName, momentArmsOnDofs));
            }
        }
    }

    void LmtMaFromMcbs::operator()() {
        inputCoordinateQueue_.subscribe();
        LmtMaFromX::doneWithSubscriptions();

        bool runCondition(true);

        do {
            GeneralisedCoordinatesFrame coordinates(inputCoordinateQueue_.pop());
            double currentTime(coordinates.time);
            if (!EndOfData::isEod(coordinates)) {
                LmtData lmts;
                vector<double> dofsValues(dofMapper_.map(coordinates.data.getQ()));

                auto timeBegin = std::chrono::high_resolution_clock::now();

                splineSet_->getLmt(lmts, dofsValues);
                auto timeEnd = std::chrono::high_resolution_clock::now();
                //std::cout << "mcbs time: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin).count() << "ms" << std::endl;

                LmtMaFromX::updateLmt(lmts, currentTime);
                for (auto dofIt(dofNamesFromSpline_.cbegin()); dofIt != dofNamesFromSpline_.cend(); ++dofIt) {
                    MomentArmsData maValues;
                    splineSet_->getMa(*dofIt, muscleNamesOnDofsMap_[*dofIt], dofsValues, maValues);
                    auto it(std::find(dofNames_.cbegin(), dofNames_.cend(), *dofIt));
                    unsigned dofIdx(std::distance(dofNames_.cbegin(), it));
                    LmtMaFromX::updateMomentArms(maValues, currentTime, dofIdx);
                }

                for (auto dofIt(dofsNamesFromOsimModel_.cbegin()); dofIt != dofsNamesFromOsimModel_.cend(); ++dofIt) {
                    auto it(std::find(dofNames_.cbegin(), dofNames_.cend(), *dofIt));
                    unsigned dofIdx(std::distance(dofNames_.cbegin(), it));
                    LmtMaFromX::updateMomentArms(defaultMomentArmsOnDofsMap_.at(*dofIt), currentTime, dofIdx);
                }
            }
            else {
                runCondition = false;
                LmtMaFromX::sendLmtEndOfData();
                LmtMaFromX::sendMaEndOfData();
            }
        } while (runCondition);
        LmtMaFromX::doneWithExecution();
    }
}