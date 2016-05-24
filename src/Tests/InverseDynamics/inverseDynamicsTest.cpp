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

#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/queues/GeneralisedCoordinatesQueue.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"
#include "ceinms/queues/ExternalTorquesQueue.h"
#include "ceinms/QueuesToInverseDynamics.h"
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"
#include "ceinms/QueueToStorageFile.h"
#include <iostream>
using std::cout;
using std::endl;
#include <OpenSim/OpenSim.h>
using OpenSim::Array;
using OpenSim::Storage;
#include <vector>
#include <memory>
#include <string>
using std::string;
#include <boost/filesystem.hpp>
using boost::filesystem::path;
using boost::filesystem::canonical;
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
    cout << std::setw(40) << "Dof" << std::setw(20) << "RMSE" << endl;
    cout << "          " << std::setfill('-') << std::setw(50) << '-' << endl;
    for (unsigned i(0); i < names.size(); ++i)
        cout << std::setfill(' ') << std::setw(40) << names[i] << std::setw(20) << errors[i] << endl;
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

    path dataDir(canonical(BASE_DIR) / "Data");
    path inputDir(dataDir / "input");
    path outputDir(dataDir / "output");
    path outputReferenceDir(dataDir / "outputReference");

    string osimModelFilename((inputDir / "model.osim").string());
    string kinematicsStorageFilename((inputDir / "ik.mot").string());
    string externalLoadsXmlFilename((inputDir / "ExtForcesSetup.xml").string());
    string grfFile((inputDir / "grf.mot").string());
    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> subject;
    OpenSim::Model model(osimModelFilename);
    OpenSim::Array<string> names;
    model.getCoordinateSet().getNames(names);
    vector<string> coordinateNames;
    ArrayConverter::toStdVector(names, coordinateNames);
    for (auto& c : coordinateNames)
        c += "_moment";

    GeneralisedCoordinatesQueue generalisedCoordinateQueue;
    MultipleExternalForcesQueue multipleExternalForcesQueue;
    ExternalTorquesQueue externalTorquesQueue;

    ceinms::Concurrency::Latch doneWithSubscriptions(4), doneWithExecution(4);

    GeneralisedCoordinatesFromStorageFile generalisedCoordinatesFromStorageFile(
        generalisedCoordinateQueue,
        doneWithSubscriptions,
        doneWithExecution,
        osimModelFilename,
        kinematicsStorageFilename,
        8);

    QueuesToInverseDynamics queuesToInverseDynamics(
        generalisedCoordinateQueue,
        multipleExternalForcesQueue,
        externalTorquesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        subject,
        osimModelFilename,
        externalLoadsXmlFilename,
        false);

    ExternalForcesFromStorageFile externalForcesFromStorageFile(
        multipleExternalForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        externalLoadsXmlFilename,
        grfFile);

    QueueToStorageFile<ExternalTorquesData> logger(
        externalTorquesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        coordinateNames,
        "id",
        outputDir.string());

    std::thread coordinateThr(std::ref(generalisedCoordinatesFromStorageFile));
    std::thread idThread(std::ref(queuesToInverseDynamics));
    std::thread externalForcesThr(std::ref(externalForcesFromStorageFile));
    std::thread loggerThr(std::ref(logger));

    coordinateThr.join();
    idThread.join();
    externalForcesThr.join();
    loggerThr.join();

    compareSto((outputDir / "id.sto").string(), (outputReferenceDir / "id.sto").string());
    getchar();
    return 0;
}