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
#include "ceinms/TrialData.h"
#include "ceinms/QueuesToTrialData.h"

#include "ceinms/BatchEvaluator.h"
#include "ceinms/FileUtils.h"

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;
#include <algorithm>
using std::end;
using std::begin;



double rmse(const std::vector<double>& v1, const std::vector<double>& v2) {

    double sum = 0;
    for (int i(0); i < v1.size(); ++i) {
        sum += (v1[i] - v2[i])*(v1[i] - v2[i]);
    }
    sum /= v1.size();
    return std::sqrt(sum);

}

template<typename NMSmodelT>
ceinms::TrialData readTrialData(ceinms::InputDataXmlReader dataLocations, NMSmodelT& mySubject, const string& emgGeneratorFile) {

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

    ceinms::QueuesToTrialData queuesToTrialData(emgQueue, lmtQueue, externalTorqueQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, "");

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
        std::cout << "ERROR: No data was read for trial " << std::endl;
        exit(EXIT_FAILURE);
    }

    ceinms::TrialData trial(queuesToTrialData.getTrialData());
    if (mySubject.hasOpenSimModel())
        trial.setOpenSimData(mySubject.getOpenSimModelFilename(), dataLocations.getExternalLoadsFile(), dataLocations.getMotionFile());
    return trial;
}




namespace ceinms {

    SimulationManager::SimulationManager(const string& setupFilename) :
        ceinmsSetup_(setupFilename),
        dataLocations_(ceinmsSetup_.getInputDataFile()),
        executionCfg_(ceinmsSetup_.getExecutionFile()),
        subjectFile_(ceinmsSetup_.getSubjectFile()),
        outputDirectory_(ceinmsSetup_.getOutputDirectory()),
        emgGeneratorFile_(ceinmsSetup_.getExcitationGeneratorFile())
    {

        SubjectXmlReader subjectReader(ceinmsSetup_.getSubjectFile());
        hasContactModel_ = subjectReader.hasContactModel();
        cout << "Contact model ";
        if (hasContactModel_) {
            cout << "found" << endl;
            contactModelCfg_.setXmlInputFile(subjectReader.getContactModelFile());
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
        ceinms::FileUtils::sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FibreLengths",
            "NormFibreLengths",
            "FibreVelocities",
            "NormFibreVelocities",
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
        std::cout << "runOpenLoopWithGenericContactModel" << endl;
        //Create queues for input data
        EmgQueue emgQueue;
        LmtQueue lmtQueue;
        ExternalTorquesQueue externalTorqueQueue;
        MultipleMomentArmsQueues momentArmsQueues;
        GeneralisedCoordinatesQueue generalisedCoordinatesQueue;
        MultipleExternalForcesQueue externalForcesQueue;
   //     ExternalTorquesQueue torquesFromIDQueue;

        //Creates queues for output data
        ScalarLogQueue logQueue;
        VecContactForcesQueue contactForcesQueue;
        VecContactForcesQueue intersegmentalForcesQueue;
        //Creates latches to sync threads
        Concurrency::Latch doneWithSubscriptionsLatch;
        Concurrency::Latch doneWithExecutionLatch;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());
        std::string osimModelFilename = mySubject.getOpenSimModelFilename();
        try {
            OpenSim::Model testModel(osimModelFilename);
            std::cout << "Found valid OpenSim model: " << osimModelFilename << std::endl;
        }
        catch (...)
        {
            std::cout << "OpenSim model not valid: " << osimModelFilename << std::endl;
            exit(EXIT_FAILURE);

        }
        // 2. define the thread connecting with the input sources
        std::cout << "-- define the threads connecting with the input sources" << endl;
        string emgFilename(dataLocations_.getExcitationsFile());
        EmgFromFile emgProducer(emgQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        ceinms::FileUtils::sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FibreLengths",
            "NormFibreLengths",
            "FibreVelocities",
            "NormFibreVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "MusclesContribution" };

        // 2b. define the thread consuming the output sources
        cout << "-- define the thread consuming the output sources" << endl;
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        cout << "-- define the model simulator" << endl;
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
        cout << "-- initialise contact model" << endl;
        auto contactModelSetup(contactModelCfg_.getContactModelSetupOsim());
        
        QueuesToJointReactionForces jointReactionForcesEstimator(
            generalisedCoordinatesQueue,
            externalForcesQueue,
            *logQueue["MuscleForces"],
            *logQueue["Torques"],
            contactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            osimModelFilename,
            dataLocations_.getExternalLoadsFile(),
            contactModelSetup.joints);

        GeneralisedCoordinatesFromStorageFile generalisedCoordinateFromStorageFile(
            generalisedCoordinatesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            osimModelFilename,
            dataLocations_.getMotionFile(),
            8);
        //2 create external forces produces
        cout << "-- Initialise external forces produces" << endl;
        ExternalForcesFromStorageFile externalForcesFromStorageFile(
            externalForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            dataLocations_.getExternalLoadsFile());

        //3 create and launch log
        cout << "-- Initialise loggers" << endl;
        QueueToStorageFile<VecContactForcesData> contactForcesLogger(
            contactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactModelSetup.joints,
            "ContactForces",
            outputDirectory_);

        /*    QueueToStorageFile<VecContactForcesData> intersegmentalForcesLogger(
                intersegmentalForcesQueue,
                doneWithSubscriptionsLatch,
                doneWithExecutionLatch,
                contactModelSetup.joints,
                "IntersegmentalForces",
                outputDirectory_);
                */
        OpenSim::Model osimModel(osimModelFilename);
        OpenSim::Array<std::string> osimCoordNames;
        osimModel.getCoordinateSet().getNames(osimCoordNames);
        std::vector<std::string> coordNames;
        ArrayConverter::toStdVector(osimCoordNames, coordNames);
        //QueueToStorageFile<ExternalTorquesData> externalTorquesLogger(
        //    torquesFromIDQueue,
        //    doneWithSubscriptionsLatch,
        //    doneWithExecutionLatch,
        //    coordNames,
        //    "ExternalTorques",
        //    outputDirectory_);

        //cout << "-- Initialise Inverse Dynamics" << endl;
        //QueuesToInverseDynamics inverseDynamicsSolver(
        //    generalisedCoordinatesQueue,
        //    externalForcesQueue,
        //    torquesFromIDQueue,
        //    doneWithSubscriptionsLatch,
        //    doneWithExecutionLatch,
        //    mySubject,
        //    osimModelFilename,
        //    dataLocations_.getExternalLoadsFile(),
        //    false);

        doneWithSubscriptionsLatch.setCount(9);
        doneWithExecutionLatch.setCount(9);

        cout << "-- Launch threads" << endl;
        std::thread contactForcesEstimatorThread(std::ref(jointReactionForcesEstimator));
        std::thread generalisedCoordinateThread(std::ref(generalisedCoordinateFromStorageFile));
        std::thread externalForcesThread(std::ref(externalForcesFromStorageFile));

        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));
        std::thread contactForcesLoggerThread(std::ref(contactForcesLogger));
        //      std::thread intersegmentalForcesLoggerThread(std::ref(intersegmentalForcesLogger));
   //     std::thread inverseDynamicsSolverThread(std::ref(inverseDynamicsSolver));
    //    std::thread externalTorquesLoggerThread(std::ref(externalTorquesLogger));

        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();
        contactForcesEstimatorThread.join();
        generalisedCoordinateThread.join();
        externalForcesThread.join();
        contactForcesLoggerThread.join();
        //     intersegmentalForcesLoggerThread.join();
   //     inverseDynamicsSolverThread.join();
//        externalTorquesLoggerThread.join();
        cout << "-- Closing" << endl;
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
        ceinms::FileUtils::sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FibreLengths",
            "NormFibreLengths",
            "FibreVelocities",
            "NormFibreVelocities",
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
    bool SimulationManager::runHybrid(bool useContactForces) {

        if (useContactForces) {
            switch (contactModelCfg_.getContactModelType()) {
            case ContactModelType::OpenSim:
                runHybridWithGenericContactModel<NMSmodelT>();
                break;
            case ContactModelType::KneeBalance:
                runHybridWithKneeBalanceContactModel<NMSmodelT>();
                break;
            }
        }
        else
            runHybrid<NMSmodelT>();
        return true;
    }


    template <typename NMSmodelT>
    bool SimulationManager::runHybridWithGenericContactModel() {

        //Create queues for input data
        EmgQueue emgQueue;
        LmtQueue lmtQueue;
        ExternalTorquesQueue externalTorqueQueue;// , torquesFromIDQueue;
        MultipleMomentArmsQueues momentArmsQueues;
        GeneralisedCoordinatesQueue generalisedCoordinatesQueue;
        MultipleExternalForcesQueue externalForcesQueue;
        
        //Creates queues for output data
        ScalarLogQueue logQueue;
        VecContactForcesQueue contactForcesQueue;

        //Creates latches to sync threads
        Concurrency::Latch doneWithSubscriptionsLatch;
        Concurrency::Latch doneWithExecutionLatch;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());
        
        std::string osimModelFilename = mySubject.getOpenSimModelFilename();
        try {
            OpenSim::Model testModel(osimModelFilename);
            std::cout << "Found valid OpenSim model: " << osimModelFilename << std::endl;
        }
        catch (...)
        {
            std::cout << "OpenSim model not valid: " << osimModelFilename << std::endl;
            exit(EXIT_FAILURE);

        }
        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EmgFromFile emgProducer(emgQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        ceinms::FileUtils::sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);
        vector<string> dataToLog = { "Activations",
            "FibreLengths",
            "NormFibreLengths",
            "FibreVelocities",
            "NormFibreVelocities",
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


        // 4. define the optimiser
        EmgInformedObjectiveFunctionSetup setup;

        executionCfg_.getMusclesToAdjust(setup.musclesToAdjust);
        executionCfg_.getMusclesToSynthesise(setup.musclesToSynthesise);
        executionCfg_.getHybridWeightings(setup.weightings.alpha, setup.weightings.beta, setup.weightings.gamma);
        executionCfg_.getDofSet(setup.dofsToTrack);
        if (setup.dofsToTrack.empty())
            setup.dofsToTrack = mySubject.getDoFNames();

        NMSmodelFrameOptimisation<NMSmodelT> errorMinimizer(mySubject, setup); 
        ModelEvaluationHybrid<NMSmodelT, NMSmodelFrameOptimisation<NMSmodelT>, LoggerOnQueues> simulator(
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


        // 4. start contact model stuff
        cout << "-- initialise contact model" << endl;
        auto contactModelSetup(contactModelCfg_.getContactModelSetupOsim());

        QueuesToJointReactionForces jointReactionForcesEstimator(
            generalisedCoordinatesQueue,
            externalForcesQueue,
            *logQueue["MuscleForces"],
            *logQueue["Torques"],
            contactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            mySubject,
            osimModelFilename,
            dataLocations_.getExternalLoadsFile(),
            contactModelSetup.joints);

        GeneralisedCoordinatesFromStorageFile generalisedCoordinateFromStorageFile(
            generalisedCoordinatesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            osimModelFilename,
            dataLocations_.getMotionFile(),
            8);
        //2 create external forces produces
        cout << "-- Initialise external forces produces" << endl;
        ExternalForcesFromStorageFile externalForcesFromStorageFile(
            externalForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            dataLocations_.getExternalLoadsFile());

        //3 create and launch log
        cout << "-- Initialise loggers" << endl;
        QueueToStorageFile<VecContactForcesData> contactForcesLogger(
            contactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactModelSetup.joints,
            "ContactForces",
            outputDirectory_);

        OpenSim::Model osimModel(osimModelFilename);
        OpenSim::Array<std::string> osimCoordNames;
        osimModel.getCoordinateSet().getNames(osimCoordNames);
        std::vector<std::string> coordNames;
        ArrayConverter::toStdVector(osimCoordNames, coordNames);

        doneWithSubscriptionsLatch.setCount(9);
        doneWithExecutionLatch.setCount(9);

        // 5. start the threads
        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));

        std::thread contactForcesEstimatorThread(std::ref(jointReactionForcesEstimator));
        std::thread generalisedCoordinateThread(std::ref(generalisedCoordinateFromStorageFile));
        std::thread externalForcesThread(std::ref(externalForcesFromStorageFile));
        std::thread contactForcesLoggerThread(std::ref(contactForcesLogger));


        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();
        contactForcesEstimatorThread.join();
        generalisedCoordinateThread.join();
        externalForcesThread.join();
        contactForcesLoggerThread.join();


        return 0;

    }

    template <typename NMSmodelT>
    bool SimulationManager::runHybridWithKneeBalanceContactModel() {
        std::cout << "runHybridWithKneeBalanceContactModel" << endl;
        //Create queues for input data
        EmgQueue emgQueue;
        LmtQueue lmtQueue;
        ExternalTorquesQueue externalTorqueQueue;
        MultipleMomentArmsQueues momentArmsQueues;

        //Creates queues for output data
        //the queues are automatically created by the LoggerOnQueues template
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
        ceinms::FileUtils::sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(
            lmtQueue, 
            momentArmsQueues, 
            doneWithSubscriptionsLatch, 
            doneWithExecutionLatch, 
            mySubject, 
            dataLocations_.getLmtFile(), 
            maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(
            externalTorqueQueue, 
            doneWithSubscriptionsLatch, 
            doneWithExecutionLatch, 
            mySubject, 
            externalTorqueFilename);
        vector<string> dataToLog = { "Activations",
            "FibreLengths",
            "NormFibreLengths",
            "FibreVelocities",
            "NormFibreVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "AdjustedEmgs",
            "MusclesContribution" };
        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory_);

        //4. define contact forces queue

            
        // 4. define the optimiser
        EmgInformedObjectiveFunctionSetup setup;

        executionCfg_.getMusclesToAdjust(setup.musclesToAdjust);
        executionCfg_.getMusclesToSynthesise(setup.musclesToSynthesise);
        executionCfg_.getHybridWeightings(setup.weightings.alpha, setup.weightings.beta, setup.weightings.gamma);
        executionCfg_.getDofSet(setup.dofsToTrack);
        if (setup.dofsToTrack.empty())
            setup.dofsToTrack = mySubject.getDoFNames();

        auto trial = readTrialData(dataLocations_, mySubject, emgGeneratorFile_);
        std::vector<unsigned> musclesToUpdate;
        mySubject.getMusclesIndexFromMusclesList(musclesToUpdate, mySubject.getMuscleNames());
        Result results(mySubject.getNoMuscles(), mySubject.getNoDoF(), trial.getEmgData().getNRows(), trial.getLmtData().getNRows());
        results.setDofNames(mySubject.getDoFNames());
        results.setMuscleNames(mySubject.getMuscleNames());

        NMSmodelFrameOptimisation<NMSmodelT> errorMinimizer(mySubject, setup);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        //save the hybrid values in a semarate file
        valuesToLog.push_back("ObjectiveFunctionComponentsAndWeightings");

        ModelEvaluationHybrid<NMSmodelT, NMSmodelFrameOptimisation<NMSmodelT>, LoggerOnQueues> simulator(
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

        QueueToStorageFile<ScalarVectorData> hybridErrorLogger(
            *(logQueue["ObjectiveFunctionComponentsAndWeightings"]),
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            { "TorqueError", "SumExcitation", "ExcitationTrackingError", "alpha", "beta", "gamma" },
            "ObjectiveFunctionComponentsAndWeightings",
            outputDirectory_
        );

        ScalarContactForcesQueue outputContactForcesQueue;
        //5. define contact forces estimator
        QueuesToKneeBalanceContactForces queuesToKneeBalanceContactForces(
        *logQueue["Torques"],
        externalTorqueQueue,
        outputContactForcesQueue,
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
            outputContactForcesQueue,
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            contactForcesNames,
            "ContactForces",
            outputDirectory_);
        doneWithSubscriptionsLatch.setCount(8);
        doneWithExecutionLatch.setCount(8);

        // 5. start the threads
        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));
        std::thread hybridErrorLoggerThread(std::ref(hybridErrorLogger));
        std::thread kneeContactModelThread(std::ref(queuesToKneeBalanceContactForces));
        std::thread kneeContactModelLoggerThread(std::ref(contactForcesLogger));
        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();
        hybridErrorLoggerThread.join();
        kneeContactModelThread.join();
        kneeContactModelLoggerThread.join();

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
        //the queues are automatically created by the LoggerOnQueues template
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
        ceinms::FileUtils::sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);
        vector<string> dataToLog = { "Activations",
            "FibreLengths",
            "NormFibreLengths",
            "FibreVelocities",
            "NormFibreVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "AdjustedEmgs",
            "MusclesContribution"};
        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory_);

        // 4. define the optimiser
        EmgInformedObjectiveFunctionSetup setup;
    
        executionCfg_.getMusclesToAdjust(setup.musclesToAdjust);
        executionCfg_.getMusclesToSynthesise(setup.musclesToSynthesise);
        executionCfg_.getHybridWeightings(setup.weightings.alpha, setup.weightings.beta, setup.weightings.gamma);
        executionCfg_.getDofSet(setup.dofsToTrack);
        if (setup.dofsToTrack.empty())
            setup.dofsToTrack = mySubject.getDoFNames();

     
        NMSmodelFrameOptimisation<NMSmodelT> errorMinimizer(mySubject, setup);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        //save the hybrid values in a separate file
        valuesToLog.push_back("ObjectiveFunctionComponentsAndWeightings");

        ModelEvaluationHybrid<NMSmodelT, NMSmodelFrameOptimisation<NMSmodelT>, LoggerOnQueues> simulator(
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

        QueueToStorageFile<ScalarVectorData> hybridErrorLogger(
            *(logQueue["ObjectiveFunctionComponentsAndWeightings"]),
            doneWithSubscriptionsLatch,
            doneWithExecutionLatch,
            { "TorqueError", "SumExcitation", "ExcitationTrackingError", "alpha", "beta", "gamma" },
            "ObjectiveFunctionComponentsAndWeightings",
            outputDirectory_
        );

        doneWithSubscriptionsLatch.setCount(6);
        doneWithExecutionLatch.setCount(6);

        // 5. start the threads
        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread simulatorThread(std::ref(simulator));
        std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));
        std::thread hybridErrorLoggerThread(std::ref(hybridErrorLogger));
        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        simulatorThread.join();
        queuesToStorageFilesThread.join();
        hybridErrorLoggerThread.join();

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

}