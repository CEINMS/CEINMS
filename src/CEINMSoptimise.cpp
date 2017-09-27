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
* Author(s): C. Pizzolato, M. Reggiani, E. Ceseracciu                        *
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

#include "ceinms/Utilities.h"
#include "ceinms/FileUtils.h"
#include "ceinms/CeinmsSetupXmlReader.h"
#include "SimulationManager.h"
#include <ctime>
#include "ceinms/CeinmsSetupXmlReader.h"
#include "ceinms/NMSmodel.h"

#include "ceinms/Tendon/StiffTendon.h"
#include "ceinms/Tendon/ElasticTendon.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"

#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Activation/PiecewiseActivation.h"
#include "ceinms/Curve.h"

#include "ceinms/EmgFromFile.h"
#include "ceinms/LmtMaFromStorageFile.h"
#include "ceinms/ExternalTorquesFromStorageFile.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"

#include "ceinms/ModelEvaluationOnline.h"
#include "ceinms/ModelEvaluationOffline.h"
#include "ceinms/ModelEvaluationHybrid.h"

//Hybrid optimiser
#include "ceinms/ErrorMinimizerAnnealing.h"
#include "ceinms/HybridWeightings.h"

#include "ceinms/LoggerOnQueues.h"
#include "ceinms/QueueToStorageFile.h"
#include "ceinms/QueuesToStorageFiles.h"
#include "ceinms/QueuesToJointReactionForces.h"
#include "ceinms/QueuesToIntersegmentalAndJointReactionForces.h"
#include "ceinms/QueuesToInverseDynamics.h"
#include "ceinms/QueuesToKneeBalanceContactForces.h"

#include "ceinms/concurrency/Latch.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/queues/VecContactForcesQueue.h"
#include "ceinms/queues/ScalarContactForcesQueue.h"
#include "ceinms/queues/EmgQueue.h"
#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/ExternalTorquesQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"
#include "ceinms/queues/LogQueue.h"
#include "ceinms/QueueToStorageFile.h"
#include "ceinms/ArrayConverter.h"

#include "ceinms/Hybrid/NMSmodelFrameOptimisation.h"
using namespace ceinms;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;


#include <iomanip>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <stdlib.h>


void printHeader() {

    cout << endl;
    cout << "+-+-+-+-+-+-+\n"
        << "|C|E|I|N|M|S|\n"
        << "+-+-+-+-+-+-+-+-+-+-+\n"
        << "|C|a|l|i|b|r|a|t|e|d|\n"
        << "+-+-+-+-+-+-+-+-+-+-+-+-+\n"
        << "|E|M|G|-|I|n|f|o|r|m|e|d|\n"
        << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
        << "|N|e|u|r|o|m|u|s|c|u|l|o|s|k|e|l|e|t|a|l|\n"
        << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
        << "|T|o|o|l|b|o|x|\n"
        << "+-+-+-+-+-+-+-+\n\n";

}

void printAuthors() {

    time_t now = std::time(0);
    tm *gmtm = std::gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year + 1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, Massimo Sartori, David Lloyd\n\n";

    cout << "Software developers: Claudio Pizzolato, Elena Ceseracciu, Monica Reggiani\n";
}

void PrintUsage()
{
    string progName = "CEINMSoptimise";
    string version =
        string(CEINMS_MAJOR_VERSION) + "." +
        string(CEINMS_MINOR_VERSION) + "." +
        string(CEINMS_PATCH_VERSION);
    cout << "\n" << progName << " version " << version << "\n";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}


template<typename NMSmodelT>
TrialData readTrialData(const string& inputDataFilename, NMSmodelT& mySubject, const string& trialId, const string& emgGeneratorFile) {

    InputDataXmlReader dataLocations(inputDataFilename);

    EmgQueue emgQueue;
    LmtQueue lmtQueue;
    ExternalTorquesQueue externalTorqueQueue;
    MultipleMomentArmsQueues momentArmsQueues;

    Concurrency::Latch doneWithSubscription;
    Concurrency::Latch doneWithExecution;

    string emgFilename(dataLocations.getExcitationsFile());
    EmgFromFile emgProducer(emgQueue, doneWithSubscription, doneWithExecution, mySubject, emgFilename, emgGeneratorFile);

    vector< string > dofNames;
    mySubject.getDoFNames(dofNames);
    vector< string > maFilename;
    sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
    LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, dataLocations.getLmtFile(), maFilename);

    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
    ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscription, doneWithExecution, mySubject, externalTorqueFilename);

    QueuesToTrialData queuesToTrialData(emgQueue, lmtQueue, externalTorqueQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, trialId);

    doneWithSubscription.setCount(4);
    doneWithExecution.setCount(4);

    // 4. start the threads
    std::thread emgProdThread(std::ref(emgProducer));
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    std::thread queuesToTrialDataThread(std::ref(queuesToTrialData));

    emgProdThread.join();
    lmtMaProdThread.join();
    externalTorquesProdThread.join();
    queuesToTrialDataThread.join();

    if (!queuesToTrialData.getTrialData().isConsistent())
    {
        std::cout << "ERROR: No data was read for trial " << trialId << std::endl;
        exit(EXIT_FAILURE);
    }

    auto trial(queuesToTrialData.getTrialData());
    if (mySubject.hasOpenSimModel())
        trial.setOpenSimData(mySubject.getOpenSimModelFilename(), dataLocations.getExternalLoadsFile(), dataLocations.getMotionFile());
    return trial;
}



void optimiseWeights() {

    NMSModelCfg::RunMode runMode = executionCfg_.getRunMode();
    switch (runMode) {

    case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOnline:
        exitFlag = runOpenLoop < NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues >(hasContactModel_);
        break;

    case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline:
        exitFlag = runOpenLoop < NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >(hasContactModel_);
        break;

    case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOnline:
        exitFlag = runOpenLoop<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues>(hasContactModel_);
        break;

    case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline:
        exitFlag = runOpenLoop<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >(hasContactModel_);
        break;

    case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOnline:
        exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues>(hasContactModel_);
        break;

    case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOffline:
        exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >(hasContactModel_);
        break;

    case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonBiSecOnline:
        exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues>(hasContactModel_);
        break;

    case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonBiSecOffline:
        exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >(hasContactModel_);
        break;

    case NMSModelCfg::HybridExponentialActivationStiffTendonOnline:
        exitFlag = runHybrid<NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>>(hasContactModel_);
        break;

    case NMSModelCfg::HybridExponentialActivationElasticTendonBiSecOnline:
        exitFlag = runHybrid<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>>(hasContactModel_);
        break;

    case NMSModelCfg::HybridPiecewiseActivationStiffTendonOnline:
        exitFlag = runHybrid<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>>(hasContactModel_);
        break;

    case NMSModelCfg::HybridPiecewiseActivationElasticTendonBiSecOnline:
        exitFlag = runHybrid<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>>(hasContactModel_);
        break;

    default:
        cout << "Implementation not available yet. Verify you XML configuration file" << endl;
        exitFlag = 1;
        break;
    } //end switch

    return exitFlag;
}

int main(int argc, char** argv) {

    printHeader();
    printAuthors();

#ifdef LOG
    cout << "Check configuration data...\n";
#endif

    string option = "";
    string setupFileName;
    if (argc < 2) {
        PrintUsage();
        return 0;
    }
    else {
        int i;
        for (i = 1; i <= (argc - 1); i++) {
            option = argv[i];

            // PRINT THE USAGE OPTIONS
            if ((option == "-help") || (option == "-h") || (option == "-Help") || (option == "-H") ||
                (option == "-usage") || (option == "-u") || (option == "-Usage") || (option == "-U")) {
                PrintUsage();
                return 0;
            }
            else if ((option == "-S") || (option == "-Setup")) {
                if (argv[i + 1] == 0) {
                    cout << "No setup file specified!" << endl;
                    PrintUsage();
                    return -1;
                }
                setupFileName = argv[i + 1];
                break;

                // Print a default setup file
            }
            else if ((option == "-PrintSetup") || (option == "-PS")) {
                if (ceinms::CeinmsSetupXmlReader::writeTemplateCeinmsSetupFile("defaultCeinmsSetupFile.xml"))
                {
                    std::cout << "Wrote template setup file to defaultCeinmsSetupFile.xml" << std::endl;
                    return 0;
                }
                else
                {
                    std::cout << "An error occurred while writing template setup file to defaultCeinmsSetupFile.xml" << std::endl;
                    return -1;
                }
            }
            else {
                cout << "Unrecognized option " << option << " on command line... Ignored" << endl;
                PrintUsage();
                return -1;
            }
        }
    }


    ceinms::SimulationManager simulation(setupFileName);
    bool flag = simulation.run();

    return flag;
}
