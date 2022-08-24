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
#include <iostream>
using std::cout;
using std::endl;
#include <sstream>
#include <filesystem>

#include <OpenSim/OpenSim.h>
using namespace OpenSim;

#include "ceinms/LmtMaFromMcbs.h"
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForceData.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"
#include "ceinms/QueueToStorageFile.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"
#include "ceinms/queues/GeneralisedCoordinatesQueue.h"
using namespace ceinms;

double calculateRMSE(Array<double> arr1, Array<double> arr2) {
    double sum(0);
    size_t sizeArr(std::min(arr1.size(), arr2.size()));
    for (unsigned i(0); i < sizeArr; ++i)
        sum += (arr1[i] - arr2[i])*(arr1[i] - arr2[i]);
    return std::sqrt(sum / static_cast<double>(sizeArr));
}

void printTable(vector<string> names, vector<double> errors) {
    cout << endl;
    cout << std::setw(20) << "Muscle" << std::setw(20) << "RMSE" << endl;
    cout << "          " << std::setfill('-') << std::setw(30) << '-' << endl;
    for (unsigned i(0); i < names.size(); ++i)
        cout << std::setfill(' ') << std::setw(20) << names[i] << std::setw(20) << errors[i] << endl;
    cout << endl;
}

void compareSto(const string& f1, const string& f2) {
    Storage sto1(f1.c_str());
    Storage sto2(f2.c_str());
    vector<double> errors;
    vector<string> names;
    Array<string> names1(sto1.getColumnLabels());
    Array<string> names2(sto2.getColumnLabels());
    for (unsigned i(0); i < names1.size(); ++i) {
        string currentName(names1.get(i));
        int idx2(names2.findIndex(currentName));
        //time column should be 0, and -1 is not found
        if (idx2 > 0) {
            Array<double> column1, column2;
            sto1.getDataColumn(currentName, column1);
            sto2.getDataColumn(currentName, column2);
            errors.emplace_back(calculateRMSE(column1, column2));
            names.emplace_back(currentName);
        }
    }

    cout << "Comparing " << f1 << " and " << f2 << endl;
    printTable(names, errors);
}

int main() {
    std::filesystem::path testDataDir = std::filesystem::path(BASE_DIR) /= "TestData";
    string ikFilename((testDataDir / "MotionData" / "InverseKinematics" / "ik.mot").string());
    string osimModelFilename((testDataDir / "Subject" / "scaled.osim").string());
    string subjectFilename((testDataDir / "Subject" / "uncalibrated.xml").string());
    string splineCoeffDir((testDataDir / "GeneratedSplines").string());
    string outputDir((testDataDir / "MotionData" / "McbsResults").string());
    boost::filesystem::create_directories(outputDir);
    string maDir((testDataDir / "MotionData" / "MuscleAnalysis").string());

    GeneralisedCoordinatesQueue generalisedCoordinatesQueue;
    LmtQueue lmtQueue;
    MultipleMomentArmsQueues momentArmsQueue;

    ceinms::Concurrency::Latch doneWithSubscriptions;
    ceinms::Concurrency::Latch doneWithExecution;

    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline> subject;
    setupSubject(subject, subjectFilename);
    doneWithSubscriptions.setCount(7);
    doneWithExecution.setCount(7);

    GeneralisedCoordinatesFromStorageFile generalisedCoordinatesFromStorage(
        generalisedCoordinatesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        osimModelFilename,
        ikFilename,
        8,
        subject.getDoFNames());
    LmtMaFromMcbs<4> lmtMaFromMcbs(generalisedCoordinatesQueue,
        lmtQueue,
        momentArmsQueue,
        doneWithSubscriptions,
        doneWithExecution,
        subject,
        splineCoeffDir);

    QueueToStorageFile<LmtData> lmtLogger(lmtQueue, doneWithSubscriptions, doneWithExecution, subject.getMuscleNames(), "_Mcbs_Length", outputDir);

    std::map<string, vector<string>> musclesOnDof;
    subject.getMuscleNamesOnDofs(musclesOnDof);
    using QueueToSto = QueueToStorageFile < vector<double> > ;
    vector< std::unique_ptr< QueueToSto>> queueToSto;
    auto dofNames(subject.getDoFNames());
    for (auto dofIt(dofNames.cbegin()); dofIt != dofNames.cend(); ++dofIt) {
        size_t dofIdx(std::distance(dofNames.cbegin(), dofIt));
        queueToSto.emplace_back(new QueueToSto(
            *momentArmsQueue.at(dofIdx),
            doneWithSubscriptions,
            doneWithExecution,
            musclesOnDof[*dofIt],
            "_Mcbs_MomentArm_" + *dofIt, outputDir));
    }

    std::thread ikReaderThr(std::ref(generalisedCoordinatesFromStorage));
    std::thread lmtMaFromMcbsThr(std::ref(lmtMaFromMcbs));
    std::thread lmtLoggerThr(std::ref(lmtLogger));

    vector<std::unique_ptr<std::thread>> maLoggersThrs;
    for (auto it(queueToSto.begin()); it != queueToSto.end(); ++it) {
        maLoggersThrs.emplace_back(new std::thread(std::ref(**it)));
    }

    ikReaderThr.join();
    lmtMaFromMcbsThr.join();
    lmtLoggerThr.join();
    for (auto& it : maLoggersThrs)
        it->join();

    //now compare results with muscle analysis data
    compareSto(outputDir + "/_Mcbs_Length.sto", maDir + "/_MuscleAnalysis_Length.sto");
    for (auto& dof : dofNames)
        compareSto(
        outputDir + "/_Mcbs_MomentArm_" + dof + ".sto",
        maDir + "/_MuscleAnalysis_MomentArm_" + dof + ".sto");

    return 0;
}