//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - March 2015
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//



#include "CeinmsSetupXmlReader.h"
#include "NMSmodel.h"

#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"

#include "Activation/ExponentialActivation.h"
#include "Activation/PiecewiseActivation.h"

#include "Curve.h"

#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "ExternalTorquesFromStorageFile.h"

#include "ModelEvaluationOnline.h" 
#include "ModelEvaluationOffline.h"
#include "ModelEvaluationHybrid.h"

//Hybrid optimiser
#include "ErrorMinimizerAnnealing.h"
#include "HybridWeightings.h"

#include "InputConnectors.h"
#include "OutputConnectors.h"

#include "LoggerOnQueues.h"
#include "QueuesToStorageFiles.h"
#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

namespace CEINMS {


    SimulationManager::SimulationManager(const string& setupFilename):
        ceinmsSetup_(setupFilename), 
        dataLocations_(ceinmsSetup_.getInputDataFile()),
        executionCfg_(ceinmsSetup_.getExecutionFile()),
        subjectFile_(ceinmsSetup_.getSubjectFile()),
        outputDirectory_(ceinmsSetup_.getOutputDirectory()),
        emgGeneratorFile_(ceinmsSetup_.getExcitationGeneratorFile())
    {

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

        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
        QueuesToStorageFiles queuesToStorageFiles(inputConnectors, outputConnectors, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
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

        // 2b. define the thread consuming the output sources
        vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques", "AdjustedEmgs" };
        QueuesToStorageFiles queuesToStorageFiles(inputConnectors, outputConnectors, mySubject, valuesToWrite, outputDirectory_);

        // 3. define the model simulator
        vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques", "AdjustedEmgs" };

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
            exitFlag = runOpenLoop < NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues > ();
            break;
        
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline: 
            exitFlag = runOpenLoop < NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >();
            break;
        
        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOnline: 
            exitFlag = runOpenLoop<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues>();
            break;
        
        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline: 
            exitFlag = runOpenLoop<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >();
            break;

        case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOnline: 
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues>();
            break;
        
        case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOffline: 
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >();
            break;
        
        case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonBiSecOnline: 
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>, ModelEvaluationOnline, LoggerOnQueues>();
            break;
        
        case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonBiSecOffline: 
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline>, ModelEvaluationOffline, LoggerOnQueues >();
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





}