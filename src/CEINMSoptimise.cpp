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
#include "ceinms/QueuesToTrialData.h"
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
#include "ceinms/TrialData.h"
#include "ceinms/Hybrid/NMSmodelFrameOptimisation.h"
#include "ceinms/BatchEvaluator.h"
#include "SimulationManager.h"
#include "ceinms/SimulatedAnnealing.h"
#include "ceinms/FileUtils.h"

#include <numeric>
#include <algorithm>
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
#include <algorithm>

#include <future>

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
        << "+-+-+-+-+-+-+-+\n";
    string progName = "CEINMSoptimise";
    string version =
        string(CEINMS_MAJOR_VERSION) + "." +
        string(CEINMS_MINOR_VERSION) + "." +
        string(CEINMS_PATCH_VERSION);
    cout << "\n" << progName << " version " << version << "\n";

}

void printAuthors() {

    time_t now = std::time(0);
    tm *gmtm = std::gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year + 1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, Massimo Sartori, David Lloyd\n";
    cout << "Software developers: Claudio Pizzolato, Elena Ceseracciu, Monica Reggiani\n";
}

void PrintUsage()
{
    string progName = "CEINMSoptimise";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}


template<typename NMSmodelT>
ceinms::TrialData readTrialData(const string& inputDataFilename, NMSmodelT& mySubject, const string& emgGeneratorFile) {

    ceinms::InputDataXmlReader dataLocations(inputDataFilename);
    ceinms::EmgQueue emgQueue;
    ceinms::LmtQueue lmtQueue;
    ceinms::ExternalTorquesQueue externalTorqueQueue;
    ceinms::MultipleMomentArmsQueues momentArmsQueues;

    ceinms::Concurrency::Latch doneWithSubscription;
    ceinms::Concurrency::Latch doneWithExecution;

    string emgFilename(dataLocations.getExcitationsFile());
    ceinms::EmgFromFile emgProducer(emgQueue, doneWithSubscription, doneWithExecution, mySubject, emgFilename, emgGeneratorFile);

    vector< string > dofNames;
    mySubject.getDoFNames(dofNames);
    vector< string > maFilename;
    ceinms::FileUtils::sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
    ceinms::LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, dataLocations.getLmtFile(), maFilename);

    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
    ceinms::ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscription, doneWithExecution, mySubject, externalTorqueFilename);

    ceinms::QueuesToTrialData queuesToTrialData(emgQueue, lmtQueue, externalTorqueQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, inputDataFilename);

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
        std::cout << "ERROR: No data was read for trial "  << std::endl;
        exit(EXIT_FAILURE);
    }

    ceinms::TrialData trial(queuesToTrialData.getTrialData());
    if (mySubject.hasOpenSimModel())
        trial.setOpenSimData(mySubject.getOpenSimModelFilename(), dataLocations.getExternalLoadsFile(), dataLocations.getMotionFile());
    return trial;
}



class HybridOptimisationManager {

public:
    HybridOptimisationManager(const std::string& setupFilename):
        setupFilename_(setupFilename),
        ceinmsSetup_(setupFilename),
        dataLocations_(ceinmsSetup_.getInputDataFile()),
        executionCfg_(ceinmsSetup_.getExecutionFile()),
        subjectFile_(ceinmsSetup_.getSubjectFile()),
        outputDirectory_(ceinmsSetup_.getOutputDirectory()),
        emgGeneratorFile_(ceinmsSetup_.getExcitationGeneratorFile()) {    
    }

    bool run() {
        if (executionCfg_.isHybrid())
            runWeightingOptimisation();
        else
            runOpenLoop();
        return true;
    }
private:
    struct HybridErrors {
        double beta, gamma;
        double trackError, torqueError;
    };

    bool runOpenLoop() {
        ceinms::SimulationManager simulationManager(setupFilename_);
        return simulationManager.run();
    }

    bool runWeightingOptimisation() {

        using NMSmodelT = ceinms::NMSmodel<ceinms::ExponentialActivation, ceinms::ElasticTendon_BiSec, ceinms::CurveMode::Online>;
        NMSmodelT subject;
        setupSubject(subject, subjectFile_, executionCfg_.getTolerance());
        ceinms::TrialData trial(readTrialData(ceinmsSetup_.getInputDataFile(), subject, emgGeneratorFile_));
        
        auto f([=](double beta, double gamma, NMSmodelT subject, ceinms::TrialData trial) {
            int nMtu = subject.getNoMuscles();
            int nDofs = subject.getNoDoF();
            ceinms::Result results(nMtu, nDofs, trial.getEmgData().getNRows(), trial.getLmtData().getNRows());
            vector<unsigned> musclesToUpdate;
            subject.getMusclesIndexFromMusclesList(musclesToUpdate, subject.getMuscleNames());
            ceinms::EmgInformedObjectiveFunctionSetup setup;
            executionCfg_.getMusclesToAdjust(setup.musclesToAdjust);
            executionCfg_.getMusclesToSynthesise(setup.musclesToSynthesise);
            executionCfg_.getHybridWeightings(setup.weightings.alpha, setup.weightings.beta, setup.weightings.gamma);
            executionCfg_.getDofSet(setup.dofsToTrack);
            if (setup.dofsToTrack.empty())
                setup.dofsToTrack = subject.getDoFNames();
            for (const auto& m : setup.musclesToAdjust) {
                auto col = trial.getEmgData().getColumn(m);
                setup.musclesToAdjustMaxExcitations.push_back(*std::max_element(col.begin(), col.end())); 
            }
            
            for (const auto& m : setup.dofsToTrack) {
                auto col = trial.getTorqueData().getColumn(m);
                setup.dofsToTrackMaxTorque.push_back(*std::max_element(col.begin(), col.end(), [](auto e1, auto e2) {
                    return std::abs(e1) < std::abs(e2);
                }));
             }
            
            setup.weightings.beta = beta;
            setup.weightings.gamma = gamma;
            ceinms::HybridEvaluator::evaluate(subject, trial, setup, musclesToUpdate, results);
            std::vector<double> torqueErrors(results.errors.getColumn("TorqueError"));
            double torqueError = std::accumulate(torqueErrors.begin(), torqueErrors.end(), 0.) / torqueErrors.size();
            std::vector<double> trackingError(results.errors.getColumn("ExcitationTrackingError"));
            double trackError = std::accumulate(trackingError.begin(), trackingError.end(), 0.) / trackingError.size();
            HybridErrors errs;
            errs.beta = beta;
            errs.gamma = gamma;
            errs.torqueError = torqueError;
            errs.trackError = trackError;
            return errs;
        });

        auto runGrid([&](vector<double> betas, vector<double> gammas) {
            std::vector<HybridErrors> values;
            for (auto beta : betas)
                for (auto gamma : gammas) {
                        values.emplace_back(f(beta, gamma, subject, trial));
                 }
            return values;
        });

        auto runGridParallel([&](vector<double> betas, vector<double> gammas) {
            int threadCount = 0;
            std::vector<std::future<HybridErrors>> futures;
            for (auto beta : betas)
                for (auto gamma : gammas) {
                    auto fut(std::async(std::launch::async, [=]() {
                        return f(beta, gamma, subject, trial); }
                    ));
                    futures.emplace_back(std::move(fut));
                    std::cout << "Creating thread " << ++threadCount << "/" << gammas.size()*betas.size() << std::endl;
                }

            std::vector<HybridErrors> values;
            for (auto& f : futures) {
                f.wait();
                std::cout << "Joining thread " << --threadCount << "/" << gammas.size()*betas.size() << std::endl;
                values.emplace_back(f.get());
            }
            return values;
        });


        auto getVec([](double min, double max, double interval) {
            vector<double> v;
            int nsteps = round((max - min) / interval);
            for (int i(0); i < nsteps; ++i) {
                v.push_back(min+i*interval);
            }
            return v;
        });
       
        double bmin = 1, bmax = 2, bdelta=1, gmin = 1, gmax = 5, gdelta=5;
        std::vector<HybridErrors> values;
        vector<double> betas = getVec(bmin, bmax,bdelta);
        vector<double> gammas = getVec(gmin, gmax,gdelta);
        values = runGridParallel(betas, gammas);
        double maxTorque = std::max_element(values.begin(), values.end(), [](auto e1, auto e2) {
            return e1.torqueError < e2.torqueError;
        })->torqueError;
        double maxTrack = std::max_element(values.begin(), values.end(), [](auto e1, auto e2) {
            return e1.trackError < e2.trackError;
        })->trackError;
        HybridErrors lowest = *std::min_element(values.begin(), values.end(), [maxTorque, maxTrack](auto e1, auto e2) {
            double v1 = std::pow(e1.torqueError / maxTorque, 2) + std::pow(e1.trackError / maxTrack, 2);
            double v2 = std::pow(e2.torqueError / maxTorque, 2) + std::pow(e2.trackError / maxTrack, 2);
            return v1 < v2;
        });
          

        cout << "bmin = " << bmin << "\n"
            << "bmax = " << bmax << "\n"
            << "gmin = " << gmin << "\n"
            << "gmax = " << gmax << "\n";

        bmin = std::max((lowest.beta - bdelta),bmin);
        bmax = std::min((lowest.beta + bdelta), bmax);
        gmin = std::max((lowest.gamma - gdelta), gmin);
        gmax = std::min((lowest.gamma + gdelta), gmax);
        
        betas = getVec(bmin, bmax, 1);
        gammas = getVec(gmin, gmax, 1);

        values = runGridParallel(betas, gammas);
        lowest = *std::min_element(values.begin(), values.end(), [maxTorque, maxTrack](auto e1, auto e2) {
            double v1 = std::pow(e1.torqueError / maxTorque, 2) + std::pow(e1.trackError / maxTrack, 2);
            double v2 = std::pow(e2.torqueError / maxTorque, 2) + std::pow(e2.trackError / maxTrack, 2);
            return v1 < v2;
        });

        cout << "lowest\nbeta = " << lowest.beta << "\ngamma = " << lowest.gamma << endl;
        cout << "f = " << std::pow(lowest.torqueError / maxTorque, 2) + std::pow(lowest.trackError / maxTrack, 2) << endl;
        
        std::cout << "Optimal values:\n"
            "alpha = " << 1 << "\n"
            "beta  = " << lowest.beta << "\n"
            "gamma  = " << lowest.gamma << "\n";
        
        executionCfg_.setHybridWeightings(1, lowest.beta, lowest.gamma);
        std::string trialId(ceinmsSetup_.getInputDataFile(false));
        size_t endpos = trialId.find(".xml");
        size_t startpos = trialId.rfind("/") + 1;
        trialId = trialId.substr(startpos, endpos - startpos);
        std::string newExecutionFilename(ceinmsSetup_.getExecutionFile(false));
        std::size_t pos = newExecutionFilename.find(".xml");
        if (pos != std::string::npos) {
            newExecutionFilename.insert(pos, "-optimised-"+trialId);
        }
        executionCfg_.print(newExecutionFilename);
        ceinmsSetup_.setExecutionFile(newExecutionFilename);
        
        std::string newSetupFilename(setupFilename_);
        pos = newSetupFilename.find(".xml");
        if (pos != std::string::npos) {
            newSetupFilename.insert(pos, "-optimised");
        }
        ceinmsSetup_.print(newSetupFilename);
        std::cout << "Running best solution\n";
        ceinms::SimulationManager simulationManager(newSetupFilename);
        return simulationManager.run();
      
    }

    ceinms::CeinmsSetupXmlReader ceinmsSetup_;
    ceinms::InputDataXmlReader dataLocations_;
    ceinms::ExecutionXmlReader executionCfg_;
    ceinms::ContactModelXmlReader contactModelCfg_;
    std::string subjectFile_;
    std::string executionFile_;
    std::string inputData_;
    std::string outputDirectory_;
    std::string emgGeneratorFile_;
    std::string setupFilename_;

};


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


    HybridOptimisationManager simulation(setupFileName);
    bool flag = simulation.run();

    return flag;
}
