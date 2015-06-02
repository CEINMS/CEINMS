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
* Author(s): C. Pizzolato                                                    *
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
#include <memory>
#include <map>
#include <algorithm>
#include <thread>
#include <string>
using std::string;
#include "boost/filesystem.hpp"
using boost::filesystem::path;
#include "ceinms/LmtMaFromMcbs.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include "ceinms/InputConnectors.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/QueueToStorageFile.h"
using namespace ceinms;

int main(){

    path testDataDir = path(BASE_DIR) /= "TestData";
    string ikFilename((testDataDir / "MotionData" / "InverseKinematics" / "ik.mot").string());
    string osimModelFilename((testDataDir / "Subject" / "scaled.osim").string());
    string subjectFilename((testDataDir / "Subject" / "uncalibrated.xml").string());
    string splineCoeffDir((testDataDir / "GeneratedSplines").string());


    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline> subject;
    setupSubject(subject, subjectFilename);
    InputConnectors inputConnectors;
    inputConnectors.doneWithSubscription.setCount(7);
    ceinms::Concurrency::Latch doneWithExecution(6);

//    GeneralisedCoordinatesQueue inputCoordinateQueue;
    GeneralisedCoordinatesFromStorageFile generalisedCoordinatesFromStorage(inputConnectors, osimModelFilename, ikFilename, subject.getDoFNames());
    LmtMaFromMcbs lmtMaFromMcbs(inputConnectors, inputConnectors.queueGeneralisedCoordinates, inputConnectors.doneWithSubscription, doneWithExecution, subject, splineCoeffDir);
    QueueToStorageFile<vector<double>> lmtLogger(inputConnectors.queueLmt, inputConnectors.doneWithSubscription, doneWithExecution, subject.getMuscleNames(), "lmt");
    
    std::map<string, vector<string>> musclesOnDof;
    subject.getMuscleNamesOnDofs(musclesOnDof);
    using QueueToSto = QueueToStorageFile < vector<double> > ;
    vector< std::unique_ptr< QueueToSto>> queueToSto;
    auto dofNames(subject.getDoFNames());
    for (auto dofIt(dofNames.cbegin()); dofIt != dofNames.cend(); ++ dofIt) {
        size_t dofIdx(std::distance(dofNames.cbegin(), dofIt));
        queueToSto.emplace_back(new QueueToSto(
            *inputConnectors.queueMomentArms.at(dofIdx), 
            inputConnectors.doneWithSubscription, 
            doneWithExecution, 
            musclesOnDof[*dofIt], 
            *dofIt + "_moment_arm"));
    }
   
    std::thread ikReaderThr(std::ref(generalisedCoordinatesFromStorage));
    std::thread lmtMaFromMcbsThr(std::ref(lmtMaFromMcbs));
    std::thread lmtLoggerThr(std::ref(lmtLogger));

    vector<std::unique_ptr<std::thread>> maLoggersThrs;
    for (auto it(queueToSto.begin()); it != queueToSto.end(); ++ it) {
        maLoggersThrs.emplace_back(new std::thread(std::ref(**it)));
    }
    
    ikReaderThr.join();
    lmtMaFromMcbsThr.join();
    lmtLoggerThr.join();
    for (auto& it : maLoggersThrs)
        it->join();
        
    return 0;
}