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

#include "ceinms/QueuesToIntersegmentalForces.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
using namespace ceinms;
using ceinms::Concurrency::Queue;
using ceinms::Concurrency::Latch;
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include <OpenSim/OpenSim.h>
#include <vector>
#include <memory>
#include "ceinms/ArrayConverter.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"
#include "ceinms/InputConnectors.h"


int main() {


    InputConnectors inputConnectors;
    inputConnectors.doneWithSubscription.setCount(4);

    QueuesToIntersegmentalForces::ContactForcesQueue outputContactForcesQueue;
    using MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    MyNMSmodel subject;

    string baseDir("C:/Users/S2849511/coding/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string osimFilename(baseDir + "model.osim");        
  
    QueuesToIntersegmentalForces contactForcesEstimator(
        inputConnectors.queueGeneralisedCoordinates,
        inputConnectors.queueExternalForces,
        outputContactForcesQueue,
        inputConnectors.doneWithSubscription,
        subject,
        osimFilename,
        externalLoadsFilename,
        { "ankle_r" });

    OpenSim::Model model(osimFilename);
    model.initSystem();
    OpenSim::ExternalLoads externalLoads(model, externalLoadsFilename);
    //1 create generalised coordinates producer
    GeneralisedCoordinatesFromStorageFile generalisedCoordinateFromStorageFile(inputConnectors, osimFilename, externalLoads.getExternalLoadsModelKinematicsFileName());
    //2 create external forces produces
    ExternalForcesFromStorageFile externalForcesFromStorageFile(inputConnectors, osimFilename, externalLoadsFilename);

    //3 create and launch log 
    auto logThread(std::async(std::launch::async, [&](){
        cout << "Logging\n";
        outputContactForcesQueue.subscribe();
        inputConnectors.doneWithSubscription.wait();
        double time(0);
        bool runCondition(true);
        while (runCondition) {
            auto contactForces(outputContactForcesQueue.pop());
            time = contactForces.time;
            if (time == inputConnectors.TimePlaceholderForEndOfData) runCondition = false;
            if (runCondition)
                cout << contactForces << endl;
        }
    }));

    //launch threads
    std::thread contactForcesEstimatorThread(std::ref(contactForcesEstimator));
    std::thread generalisedCoordinateThread(std::ref(generalisedCoordinateFromStorageFile));
    std::thread externalForcesThread(std::ref(externalForcesFromStorageFile));


    contactForcesEstimatorThread.join();
    generalisedCoordinateThread.join();
    externalForcesThread.join();
    logThread.wait();

}