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

#include <ctime>
#include <numeric>
#include <algorithm>
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <iomanip>
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <stdlib.h>
#include <future>
#include <utility>
#include <thread>

#include "ceinms/Utilities.h"
#include "ceinms/FileUtils.h"
#include "ceinms/CeinmsSetupXmlReader.h"
#include "SimulationManager.h"

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

#include "rtb/concurrency/Concurrency.h"

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
    cout << "Copyright (C) " << string(__DATE__) << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, Massimo Sartori, David Lloyd\n";
    cout << "Software developers: Claudio Pizzolato, Elena Ceseracciu, Monica Reggiani, Matthew Hambly\n";
}

void PrintUsage() {
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

    rtb::Concurrency::Latch doneWithSubscription;
    rtb::Concurrency::Latch doneWithExecution;

    string emgFilename(dataLocations.getExcitationsFile());
    ceinms::EmgFromFile emgProducer(emgQueue, doneWithSubscription, doneWithExecution, mySubject, emgFilename, emgGeneratorFile);

    vector<string> dofNames;
    mySubject.getDoFNames(dofNames);
    vector<string> maFilename;
    ceinms::FileUtils::sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
    ceinms::LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, dataLocations.getLmtFile(), maFilename);

    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
    ceinms::ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscription, doneWithExecution, mySubject, externalTorqueFilename);

    ceinms::QueuesToTrialData queuesToTrialData(emgQueue, lmtQueue, externalTorqueQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, inputDataFilename);

    doneWithSubscription.setCount(4);
    doneWithExecution.setCount(4);

    // start the threads
    std::thread emgProdThread(std::ref(emgProducer));
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    std::thread queuesToTrialDataThread(std::ref(queuesToTrialData));

    emgProdThread.join();
    lmtMaProdThread.join();
    externalTorquesProdThread.join();
    queuesToTrialDataThread.join();

    if (!queuesToTrialData.getTrialData().isConsistent()) {
        cout << "ERROR: No data was read for trial "  << endl;
        exit(EXIT_FAILURE);
    }

    ceinms::TrialData trial(queuesToTrialData.getTrialData());
    if (mySubject.hasOpenSimModel())
        trial.setOpenSimData(mySubject.getOpenSimModelFilename(), dataLocations.getExternalLoadsFile(), dataLocations.getMotionFile());
    return trial;
}

class HybridOptimisationManager {

public:
    HybridOptimisationManager(const string& setupFilename):
        setupFilename_(setupFilename),
        ceinmsSetup_(setupFilename),
        dataLocations_(ceinmsSetup_.getInputDataFile()),
        executionCfg_(ceinmsSetup_.getExecutionFile()),
        subjectFile_(ceinmsSetup_.getSubjectFile()),
        outputDirectory_(ceinmsSetup_.getOutputDirectory()),
        emgGeneratorFile_(ceinmsSetup_.getExcitationGeneratorFile()) {  

        //These are optional in the setup file; keep the defaults below when absent
        bmin_ = ceinmsSetup_.getBetaMin(bmin_);
        bmax_ = ceinmsSetup_.getBetaMax(bmax_);
        bdelta_ = ceinmsSetup_.getBetaDelta(bdelta_);
        gmin_ = ceinmsSetup_.getGammaMin(gmin_);
        gmax_ = ceinmsSetup_.getGammaMax(gmax_);
        gdelta_ = ceinmsSetup_.getGammaDelta(gdelta_);
    }

    bool run() {
        //Both return SimulationManager::run()'s exit code (0 on success), so
        //propagate it. Returning true unconditionally made every successful run
        //exit with code 1, which reads as a failure to any calling script.
        if (executionCfg_.isHybrid())
            return runWeightingOptimisation();
        return runOpenLoop();
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
            vector<double> torqueErrors(results.errors.getColumn("TorqueError"));
            double torqueError = std::accumulate(torqueErrors.begin(), torqueErrors.end(), 0.) / torqueErrors.size();
            vector<double> trackingError(results.errors.getColumn("ExcitationTrackingError"));
            double trackError = std::accumulate(trackingError.begin(), trackingError.end(), 0.) / trackingError.size();
            HybridErrors errs;
            errs.beta = beta;
            errs.gamma = gamma;
            errs.torqueError = torqueError;
            errs.trackError = trackError;
            return errs;
        });

        auto runGrid([&](vector<double> betas, vector<double> gammas) {
            vector<HybridErrors> values;
            for (auto beta : betas)
                for (auto gamma : gammas) {
                        values.emplace_back(f(beta, gamma, subject, trial));
                }
            return values;
        });

        auto runGridParallel([&](vector<double> betas, vector<double> gammas) {
            vector<std::pair<double, double>> gridPoints;
            for (auto beta : betas)
                for (auto gamma : gammas)
                    gridPoints.emplace_back(beta, gamma);

            const unsigned maxConcurrentThreads = std::max(1u, std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 1u);

            vector<HybridErrors> values;
            values.reserve(gridPoints.size());
            size_t submitted = 0;
            while (submitted < gridPoints.size()) {
                size_t batchSize = std::min<size_t>(maxConcurrentThreads, gridPoints.size() - submitted);
                vector<std::future<HybridErrors>> futures;
                for (size_t i = 0; i < batchSize; ++i) {
                    double beta = gridPoints[submitted + i].first;
                    double gamma = gridPoints[submitted + i].second;
                    futures.emplace_back(std::async(std::launch::async, [=]() {
                        return f(beta, gamma, subject, trial); }
                    ));
                }
                for (auto& fut : futures) {
                    values.emplace_back(fut.get());
                }
                submitted += batchSize;
                cout << "Completed " << submitted << "/" << gridPoints.size() << " grid points (using up to " << maxConcurrentThreads << " threads)" << endl;
            }
            return values;
        });


        //Walks min..max inclusive. nsteps counts intervals, so the loop runs to
        //nsteps to include max itself rather than stopping one step short. A
        //degenerate range still yields a single point: an empty grid would leave
        //the min_element/max_element calls below dereferencing end().
        auto getVec([](double min, double max, double interval) {
            vector<double> v;
            if (!(interval > 0.) || !(max > min)) {
                v.push_back(min);
                return v;
            }
            const int nsteps = static_cast<int>(round((max - min) / interval));
            for (int i(0); i <= nsteps; ++i)
                v.push_back(std::min(min + i * interval, max));
            return v;
        });
       
        double bmin = bmin_, bmax = bmax_, bdelta = bdelta_, gmin = gmin_, gmax = gmax_, gdelta = gdelta_;
        vector<HybridErrors> values;
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
        
        cout << "Optimal values:\n"
            "alpha = " << 1 << "\n"
            "beta  = " << lowest.beta << "\n"
            "gamma  = " << lowest.gamma << "\n";
        
        executionCfg_.setHybridWeightings(1, lowest.beta, lowest.gamma);
        string trialId(ceinmsSetup_.getInputDataFile(false));
        size_t endpos = trialId.find(".xml");
        //Look for both separators: a Windows path would otherwise keep its
        //directory components in the generated output filename.
        size_t seppos = trialId.find_last_of("/\\");
        size_t startpos = (seppos == string::npos) ? 0 : seppos + 1;
        trialId = trialId.substr(startpos, endpos - startpos);
        string newExecutionFilename(ceinmsSetup_.getExecutionFile(false));
        std::size_t pos = newExecutionFilename.find(".xml");
        if (pos != string::npos) {
            newExecutionFilename.insert(pos, "-optimised-"+trialId);
        }
        executionCfg_.print(newExecutionFilename);
        ceinmsSetup_.setExecutionFile(newExecutionFilename);
        
        string newSetupFilename(setupFilename_);
        pos = newSetupFilename.find(".xml");
        if (pos != string::npos) {
            newSetupFilename.insert(pos, "-optimised");
        }
        ceinmsSetup_.print(newSetupFilename);
        cout << "Running best solution\n";
        ceinms::SimulationManager simulationManager(newSetupFilename);
        return simulationManager.run();
    }

    ceinms::CeinmsSetupXmlReader ceinmsSetup_;
    ceinms::InputDataXmlReader dataLocations_;
    ceinms::ExecutionXmlReader executionCfg_;
    ceinms::ContactModelXmlReader contactModelCfg_;
    string subjectFile_;
    string executionFile_;
    string inputData_;
    string outputDirectory_;
    string emgGeneratorFile_;
    string setupFilename_;
    double bmin_ = 1;
    double bmax_ = 2;
    double bdelta_ = 1;
    double gmin_ = 1;
    double gmax_ = 5;
    double gdelta_ = 5;
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
                if (ceinms::CeinmsSetupXmlReader::writeTemplateCeinmsSetupFile("defaultCeinmsSetupFile.xml")) {
                    cout << "Wrote template setup file to defaultCeinmsSetupFile.xml" << endl;
                    return 0;
                }
                else {
                    cout << "An error occurred while writing template setup file to defaultCeinmsSetupFile.xml" << endl;
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
