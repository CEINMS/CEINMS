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
#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

namespace ceinms {

    SimulationManager::SimulationManager(const string& setupFilename) :
        ceinmsSetup_(setupFilename),
        dataLocations_(ceinmsSetup_.getInputDataFile()),
        executionCfg_(ceinmsSetup_.getExecutionFile()),
        subjectFile_(ceinmsSetup_.getSubjectFile()),
        outputDirectory_(ceinmsSetup_.getOutputDirectory()),
        emgGeneratorFile_(ceinmsSetup_.getExcitationGeneratorFile()),
        hasContactModel_(ceinmsSetup_.hasContactModel())
    {

        cout << "Contact model ";
        if (hasContactModel_) {
            cout << "found" << endl;
            contactModelCfg_.setXmlInputFile(ceinmsSetup_.getContactModelFile());
        }
        else
            cout << " not found" << endl;
    }

    template <typename NMSmodelT, template <typename, typename> class ModelEvaluatorT, typename LoggerT>
    bool SimulationManager::runOpenLoop(bool useContactForces) {

        if (useContactForces) {
            switch (contactModelCfg_.getContactModelType()) {
            case ContactModelType::OpenSim:
                runOpenLoopWithGenericContactModel < NMSmodelT, ModelEvaluatorT, LoggerT >();
                break;
            case ContactModelType::KneeBalance:
                runOpenLoopWithKneeBalanceContactModel < NMSmodelT, ModelEvaluatorT, LoggerT >();
                break;
            }
        }
        else
            runOpenLoop< NMSmodelT, ModelEvaluatorT, LoggerT >();
        return true;
    }

    template <typename NMSmodelT, template <typename, typename> class ModelEvaluatorT, typename LoggerT>
    bool SimulationManager::runOpenLoopWithKneeBalanceContactModel() {

        //Create queues for input data
        EmgQueue emgQueue;
        LmtQueue lmtQueue;
        ExternalTorquesQueue externalTorqueQueue;
        MultipleMomentArmsQueues momentArmsQueues;

        //Creates queues for output data
        ScalarLogQueue logQueue;
        ScalarContactForcesQueue contactForcesQueue;

        //Creates latches to sync threads
        Concurrency::Latch doneWithSubscriptionsLatch;
        Concurrency::Latch doneWithExecutionLatch;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EmgFromFile emgProducer(emgQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FiberLenghts",
            "NormFiberLengths",
            "FiberVelocities",
            "NormFiberVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "MusclesContribution" };

        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        ModelEvaluatorT<NMSmodelT, LoggerT>  simulator(
            emgQueue,
            lmtQueue,
            externalTorqueQueue,
            momentArmsQueues,
            logQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            valuesToLog);

        //4. define contact forces queue
        ScalarContactForcesQueue outputContactForcesQueue;
        //5. define contact forces estimator
        QueuesToKneeBalanceContactForces queuesToKneeBalanceContactForces(
            *logQueue["Torques"],
            externalTorqueQueue,
            contactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactModelCfg_.getContactModelSetupKneeBalance(),
            mySubject);

        const vector<string> contactForcesNames{
            contactModelCfg_.getContactModelSetupKneeBalance().condyle1.name + "_force",
            contactModelCfg_.getContactModelSetupKneeBalance().condyle2.name + "_force",
        };

        //6. define logging for contact forces
        QueueToStorageFile<ScalarVectorData> contactForcesLogger(
            contactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactForcesNames,
            "ContactForces",
            outputDirectory_);

        doneWithSubscriptionsLatch.setCount(7);
        doneWithExecutionLatch.setCount(7);

        // 4. start the threads
        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));
        std::thread kneeContactModelThread(std::ref(queuesToKneeBalanceContactForces));
        std::thread kneeContactModelLoggerThread(std::ref(contactForcesLogger));

        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();
        kneeContactModelThread.join();
        kneeContactModelLoggerThread.join();

        return 0;
    }

    template <typename NMSmodelT, template <typename, typename> class ModelEvaluatorT, typename LoggerT>
    bool SimulationManager::runOpenLoopWithGenericContactModel() {

        //Create queues for input data
        EmgQueue emgQueue;
        LmtQueue lmtQueue;
        ExternalTorquesQueue externalTorqueQueue;
        MultipleMomentArmsQueues momentArmsQueues;
        GeneralisedCoordinatesQueue generalisedCoordinatesQueue;
        MultipleExternalForcesQueue externalForcesQueue;
        ExternalTorquesQueue torquesFromIDQueue;

        //Creates queues for output data
        ScalarLogQueue logQueue;
        VecContactForcesQueue contactForcesQueue;
        VecContactForcesQueue intersegmentalForcesQueue;
        //Creates latches to sync threads
        Concurrency::Latch doneWithSubscriptionsLatch;
        Concurrency::Latch doneWithExecutionLatch;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EmgFromFile emgProducer(emgQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FiberLenghts",
            "NormFiberLengths",
            "FiberVelocities",
            "NormFiberVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "MusclesContribution" };

        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        ModelEvaluatorT<NMSmodelT, LoggerT>  simulator(
            emgQueue,
            lmtQueue,
            externalTorqueQueue,
            momentArmsQueues,
            logQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            valuesToLog);

        // 4. start contact model stuff

        auto contactModelSetup(contactModelCfg_.getContactModelSetupOsim());
        QueuesToIntersegmentalAndJointReactionForces jointReactionForcesEstimator(
            generalisedCoordinatesQueue,
            externalForcesQueue,
            *logQueue["MuscleForces"],
            *logQueue["Torques"],
            contactForcesQueue,
            intersegmentalForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            contactModelSetup.osimModel,
            contactModelSetup.externalLoads,
            contactModelSetup.joints);

        GeneralisedCoordinatesFromStorageFile generalisedCoordinateFromStorageFile(
            generalisedCoordinatesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactModelSetup.osimModel,
            contactModelSetup.motion,
            8);
        //2 create external forces produces
        ExternalForcesFromStorageFile externalForcesFromStorageFile(
            externalForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactModelSetup.osimModel,
            contactModelSetup.externalLoads);

        //3 create and launch log
        QueueToStorageFile<VecContactForcesData> contactForcesLogger(
            contactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactModelSetup.joints,
            "ContactForces",
            outputDirectory_);

        QueueToStorageFile<VecContactForcesData> intersegmentalForcesLogger(
            intersegmentalForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactModelSetup.joints,
            "IntersegmentalForces",
            outputDirectory_);

        OpenSim::Model osimModel(contactModelSetup.osimModel);
        OpenSim::Array<std::string> osimCoordNames;
        osimModel.getCoordinateSet().getNames(osimCoordNames);
        std::vector<std::string> coordNames;
        ArrayConverter::toStdVector(osimCoordNames, coordNames);
        QueueToStorageFile<ExternalTorquesData> externalTorquesLogger(
            torquesFromIDQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            coordNames,
            "ExternalTorques",
            outputDirectory_);

        QueuesToInverseDynamics inverseDynamicsSolver(
            generalisedCoordinatesQueue,
            externalForcesQueue,
            torquesFromIDQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            contactModelSetup.osimModel,
            contactModelSetup.externalLoads,
            false);

        doneWithSubscriptionsLatch.setCount(12);
        doneWithExecutionLatch.setCount(12);

        //launch threads
        std::thread contactForcesEstimatorThread(std::ref(jointReactionForcesEstimator));
        std::thread generalisedCoordinateThread(std::ref(generalisedCoordinateFromStorageFile));
        std::thread externalForcesThread(std::ref(externalForcesFromStorageFile));

        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));
        std::thread contactForcesLoggerThread(std::ref(contactForcesLogger));
        std::thread intersegmentalForcesLoggerThread(std::ref(intersegmentalForcesLogger));
        std::thread inverseDynamicsSolverThread(std::ref(inverseDynamicsSolver));
        std::thread externalTorquesLoggerThread(std::ref(externalTorquesLogger));

        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();
        contactForcesEstimatorThread.join();
        generalisedCoordinateThread.join();
        externalForcesThread.join();
        contactForcesLoggerThread.join();
        intersegmentalForcesLoggerThread.join();
        inverseDynamicsSolverThread.join();
        externalTorquesLoggerThread.join();

        return 0;
    }

    template <typename NMSmodelT, template <typename, typename> class ModelEvaluatorT, typename LoggerT>
    bool SimulationManager::runOpenLoop() {

        //Create queues for input data
        EmgQueue emgQueue;
        LmtQueue lmtQueue;
        ExternalTorquesQueue externalTorqueQueue;
        MultipleMomentArmsQueues momentArmsQueues;

        //Creates queues for output data
        ScalarLogQueue logQueue;

        //Creates latches to sync threads
        Concurrency::Latch doneWithSubscriptionsLatch;
        Concurrency::Latch doneWithExecutionLatch;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EmgFromFile emgProducer(emgQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FiberLenghts",
            "NormFiberLengths",
            "FiberVelocities",
            "NormFiberVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "MusclesContribution" };
        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        ModelEvaluatorT<NMSmodelT, LoggerT>  simulator(
            emgQueue,
            lmtQueue,
            externalTorqueQueue,
            momentArmsQueues,
            logQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            valuesToLog);

        doneWithSubscriptionsLatch.setCount(5);
        doneWithExecutionLatch.setCount(5);

        // 4. start the threads
        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));

        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();

        return 0;
    }

    template <typename NMSmodelT>
    bool SimulationManager::runHybrid() {

        //Create queues for input data
        EmgQueue emgQueue;
        LmtQueue lmtQueue;
        ExternalTorquesQueue externalTorqueQueue;
        MultipleMomentArmsQueues momentArmsQueues;

        //Creates queues for output data
        ScalarLogQueue logQueue;

        //Creates latches to sync threads
        Concurrency::Latch doneWithSubscriptionsLatch;
        Concurrency::Latch doneWithExecutionLatch;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EmgFromFile emgProducer(emgQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);
        vector<string> dataToLog = { "Activations",
            "FiberLenghts",
            "NormFiberLengths",
            "FiberVelocities",
            "NormFiberVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "AdjustedEmgs",
            "MusclesContribution" };
        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;

        doneWithSubscriptionsLatch.setCount(5);
        doneWithExecutionLatch.setCount(5);

        // 4. define the optimiser
        typedef Hybrid::ErrorMinimizerAnnealing<NMSmodelT> MyErrorMinimizer;
        MyErrorMinimizer errorMinimizer(mySubject);
        HybridWeightings weightings;
        executionCfg_.getHybridWeightings(weightings.alpha, weightings.beta, weightings.gamma);
        errorMinimizer.setWeightings(weightings);
        vector<string> toSynth, toAdjust;
        executionCfg_.getMusclesToSynthesise(toSynth);
        executionCfg_.getMusclesToAdjust(toAdjust);
        errorMinimizer.setMusclesNamesWithEmgToPredict(toSynth);
        errorMinimizer.setMusclesNamesWithEmgToTrack(toAdjust);
        double rt, t, epsilon;
        unsigned noEpsilon, ns, nt, maxNoEval;
        executionCfg_.getAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
        errorMinimizer.setAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);

        ModelEvaluationHybrid<NMSmodelT, MyErrorMinimizer, LoggerOnQueues> simulator(
            emgQueue,
            lmtQueue,
            externalTorqueQueue,
            momentArmsQueues,
            logQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            errorMinimizer,
            valuesToLog);

        // 5. start the threads
        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));

        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();

        return 0;
    }

    bool SimulationManager::run() {

        bool exitFlag(0);

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
            exitFlag = runHybrid<NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>>();
            break;

        case NMSModelCfg::HybridExponentialActivationElasticTendonBiSecOnline:
            exitFlag = runHybrid<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>>();
            break;

        case NMSModelCfg::HybridPiecewiseActivationStiffTendonOnline:
            exitFlag = runHybrid<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>>();
            break;

        case NMSModelCfg::HybridPiecewiseActivationElasticTendonBiSecOnline:
            exitFlag = runHybrid<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>>();
            break;

        default:
            cout << "Implementation not available yet. Verify you XML configuration file" << endl;
            exitFlag = 1;
            break;
        } //end switch

        return exitFlag;
    }

    void SimulationManager::sortMaFilenames(const std::map<std::string, std::string>& maMap, const std::vector< std::string >& dofNames, std::vector< std::string >& maDataFilenames)
    {
        int currentDof = 0;
        for (auto& it : dofNames)
        {
            try
            {
                maDataFilenames.push_back(maMap.at(it));
            }
            catch (std::out_of_range)
            {
                std::cerr << "Could not find moment arm file for " << it << " degree of freedom" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}