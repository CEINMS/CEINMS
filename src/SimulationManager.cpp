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


#include "ceinms/EMGFromFile.h"
#include "ceinms/LmtMaFromStorageFile.h"
#include "ceinms/ExternalTorquesFromStorageFile.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"

#include "ModelEvaluationOnline.h"
#include "ModelEvaluationOffline.h"
#include "ModelEvaluationHybrid.h"

//Hybrid optimiser
#include "ceinms/ErrorMinimizerAnnealing.h"
#include "ceinms/HybridWeightings.h"

#include "ceinms/InputConnectors.h"
#include "ceinms/OutputConnectors.h"

#include "ceinms/LoggerOnQueues.h"
#include "ceinms/QueuesToStorageFiles.h"
#include "ceinms/QueuesToJointReactionForces.h"
#include "ceinms/QueuesToKneeBalanceContactForces.h"

#include "ceinms/queues/VecContactForcesQueue.h"
#include "ceinms/queues/ScalarContactForcesQueue.h"
#include "ceinms/QueueToStorageFile.h"

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

namespace ceinms {


    SimulationManager::SimulationManager(const string& setupFilename):
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

        InputConnectors inputConnectors;
        OutputConnectors outputConnectors;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EMGFromFile emgProducer(inputConnectors, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(inputConnectors, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(inputConnectors, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FiberLenghts",
            "NormFiberLengths",
            "FiberVelocities",
            "NormFiberVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques" };


        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(inputConnectors, outputConnectors, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        ModelEvaluatorT<NMSmodelT, LoggerT>  simulator(inputConnectors, outputConnectors, mySubject, valuesToLog);

        //4. define contact forces queue
        ScalarContactForcesQueue outputContactForcesQueue;
        //5. define contact forces estimator
        QueuesToKneeBalanceContactForces queuesToKneeBalanceContactForces(
            *outputConnectors.logQueues["Torques"],
            inputConnectors.queueExternalTorques,
            outputContactForcesQueue,
            inputConnectors.doneWithSubscription,
            contactModelCfg_.getContactModelSetupKneeBalance(),
            mySubject);

        const vector<string> contactForcesNames{
            contactModelCfg_.getContactModelSetupKneeBalance().condyle1.name+"_force",
            contactModelCfg_.getContactModelSetupKneeBalance().condyle2.name+ "_force",
        };

        //6. define logging for contact forces
        QueueToStorageFile<std::vector<double>> contactForcesLogger(
            outputContactForcesQueue,
            inputConnectors.doneWithSubscription,
            outputConnectors.doneWithExecution,
            contactForcesNames,
            "ContactForces",
            outputDirectory_);
                
        inputConnectors.doneWithSubscription.setCount(7);
        outputConnectors.doneWithExecution.setCount(3);

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
        
        InputConnectors inputConnectors;
        OutputConnectors outputConnectors;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EMGFromFile emgProducer(inputConnectors, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(inputConnectors, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(inputConnectors, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
            "FiberLenghts",
            "NormFiberLengths",
            "FiberVelocities",
            "NormFiberVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques" };
        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(inputConnectors, outputConnectors, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        ModelEvaluatorT<NMSmodelT, LoggerT>  simulator(inputConnectors, outputConnectors, mySubject, valuesToLog);

        VecContactForcesQueue outputJointReactionForcesQueue;

        // 4. start contact model stuff
        auto contactModelSetup(contactModelCfg_.getContactModelSetupOsim());
        QueuesToJointReactionForces jointReactionForcesEstimator(inputConnectors.queueGeneralisedCoordinates,
            inputConnectors.queueExternalForces,
            *outputConnectors.logQueues["MuscleForces"],
            *outputConnectors.logQueues["Torques"],
            outputJointReactionForcesQueue,
            inputConnectors.doneWithSubscription,
            mySubject,
            contactModelSetup.osimModel,
            contactModelSetup.externalLoads,
            contactModelSetup.joints);
            
        GeneralisedCoordinatesFromStorageFile generalisedCoordinateFromStorageFile(inputConnectors, contactModelSetup.osimModel, contactModelSetup.motion);
        //2 create external forces produces
        ExternalForcesFromStorageFile externalForcesFromStorageFile(inputConnectors, contactModelSetup.osimModel, contactModelSetup.externalLoads);

        //3 create and launch log 
        QueueToStorageFile<std::vector<SimTK::Vec3>> contactForcesLogger(
            outputJointReactionForcesQueue,
            inputConnectors.doneWithSubscription,
            outputConnectors.doneWithExecution,
            contactModelSetup.joints,
            "ContactForces",
            outputDirectory_);

        inputConnectors.doneWithSubscription.setCount(9);
        outputConnectors.doneWithExecution.setCount(3);

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


    template <typename NMSmodelT, template <typename, typename> class ModelEvaluatorT, typename LoggerT>
    bool SimulationManager::runOpenLoop() {

        //Monica probably won't like this
        InputConnectors inputConnectors;
        OutputConnectors outputConnectors;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_, executionCfg_.getTolerance());

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EMGFromFile emgProducer(inputConnectors, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(inputConnectors, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(inputConnectors, mySubject, externalTorqueFilename);

        vector<string> dataToLog = { "Activations",
                                     "FiberLenghts",
                                     "NormFiberLengths",
                                     "FiberVelocities",
                                     "NormFiberVelocities",
                                     "PennationAngles",
                                     "MuscleForces",
                                     "Torques" };
        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(inputConnectors, outputConnectors, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;
        ModelEvaluatorT<NMSmodelT, LoggerT>  simulator(inputConnectors, outputConnectors, mySubject, valuesToLog);

        inputConnectors.doneWithSubscription.setCount(5);
        outputConnectors.doneWithExecution.setCount(2);

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
        // 1. define the model
        InputConnectors inputConnectors;
        OutputConnectors outputConnectors;

        NMSmodelT mySubject;
        setupSubject(mySubject, subjectFile_);

        // 2. define the thread connecting with the input sources
        string emgFilename(dataLocations_.getExcitationsFile());
        EMGFromFile emgProducer(inputConnectors, mySubject, emgFilename, emgGeneratorFile_);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations_.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(inputConnectors, mySubject, dataLocations_.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations_.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(inputConnectors, mySubject, externalTorqueFilename);
        vector<string> dataToLog = { "Activations",
            "FiberLenghts",
            "NormFiberLengths",
            "FiberVelocities",
            "NormFiberVelocities",
            "PennationAngles",
            "MuscleForces",
            "Torques",
            "AdjustedEmgs" };
        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = dataToLog;
        QueuesToStorageFiles queuesToStorageFiles(inputConnectors, outputConnectors, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = dataToLog;

        inputConnectors.doneWithSubscription.setCount(5);
        outputConnectors.doneWithExecution.setCount(2);

        // 4. define the optimiser
        typedef Hybrid::ErrorMinimizerAnnealing<NMSmodelT> MyErrorMinimizer;
        MyErrorMinimizer errorMinimizer(mySubject);
        HybridWeightings weightings;
        executionCfg_.getHybridWeightings(weightings.alpha, weightings.beta, weightings.gamma);
        errorMinimizer.setWeightings(weightings);
        vector<string> toPredict, toTrack;
        executionCfg_.getMusclesToPredict(toPredict);
        executionCfg_.getMusclesToTrack(toTrack);
        errorMinimizer.setMusclesNamesWithEmgToPredict(toPredict);
        errorMinimizer.setMusclesNamesWithEmgToTrack(toTrack);
        double rt, t, epsilon;
        unsigned noEpsilon, ns, nt, maxNoEval;
        executionCfg_.getAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
        errorMinimizer.setAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);

        ModelEvaluationHybrid<NMSmodelT, MyErrorMinimizer, LoggerOnQueues> simulator(inputConnectors, outputConnectors, mySubject, errorMinimizer, valuesToLog);

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
            exitFlag = runOpenLoop < NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues > (hasContactModel_);
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
