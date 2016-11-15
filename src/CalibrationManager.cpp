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

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <list>
using std::list;
#include <vector>
using std::vector;
#include <thread>
#include <stdlib.h>

#include "ceinms/NMSmodel.h"

#include "ceinms/Tendon/StiffTendon.h"
#include "ceinms/Tendon/ElasticTendon.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Activation/PiecewiseActivation.h"
#include "ceinms/MinTorqueError.h"
#include "ceinms/SumKneeContactForces.h"
#include "ceinms/SumGenericContactForces.h"
#include "ceinms/MinTorqueErrorAndContactForces.h"
#include "ceinms/ComposedObjectiveFunction.h"

#include "ceinms/SimulatedAnnealing.h"
#include "ceinms/NMSmodelSystem.h"
#include "ceinms/Curve.h"
#include "ceinms/InputDataXmlReader.h"
#include "ceinms/EmgFromFile.h"
#include "ceinms/ExternalTorquesFromStorageFile.h"
#include "ceinms/LmtMaFromStorageFile.h"
#include "ceinms/ObjectiveFunctionSetup.h"

#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/queues/EmgQueue.h"
#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/ExternalTorquesQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"
#include "ceinms/QueuesToTrialData.h"

#include "XmlWriter.h"
#include "ceinms/FileUtils.h"

namespace ceinms {

    CalibrationManager::CalibrationManager(const string& setupFilename) :
        ceinmsCalibrationSetup_(setupFilename),
        uncalibratedSubjectFile_(ceinmsCalibrationSetup_.getSubjectFile()),
        calibrationFile_(ceinmsCalibrationSetup_.getCalibrationFile()),
        calibrationXmlReader_(ceinmsCalibrationSetup_.getCalibrationFile()),
        calibratedSubjectFile_(ceinmsCalibrationSetup_.getOutputSubjectFile()),
        emgGeneratorFile_(ceinmsCalibrationSetup_.getExcitationGeneratorFile()){

        SubjectXmlReader subjectReader(ceinmsCalibrationSetup_.getSubjectFile());
        hasContactModel_ = subjectReader.hasContactModel();
        cout << "Contact model ";
        if (hasContactModel_) {
            cout << "found" << endl;
            contactModelCfg_.setXmlInputFile(subjectReader.getContactModelFile());
        }
        else
            cout << " not found" << endl;

        calibrationXmlReader_.getCalibrationTrials(calibrationTrialIDs_);

    }

    template <typename NMSmodelT>
    bool CalibrationManager::runOpenLoop() {
        bool exitflag(false);
        NMSmodelT mySubject;
        setupSubject(mySubject, uncalibratedSubjectFile_, calibrationXmlReader_.getTolerance());

        vector<TrialData> trials;
        //Somewhere in the following lines, CEINMScalibrate crashes.
        for (list<string>::iterator trialIt = calibrationTrialIDs_.begin(); trialIt != calibrationTrialIDs_.end(); ++trialIt) {
            //
            /*try {
                cout << "\nPrinted before trials.push_back" << endl;*/
                trials.push_back(readTrialData(*trialIt, mySubject, *trialIt, emgGeneratorFile_));
                /*cout << "\nPrinted after trials.push_back" << endl;
                system("pause");
                }
            catch (char param) {
                cout << "Error!" << endl;
                //exit(EXIT_FAILURE);
            }*/
        }

        CalibrationStep currentCalibrationStep;
        while (calibrationXmlReader_.popNextCalibrationStep(currentCalibrationStep)) {
            std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
            switch (currentCalibrationStep.getStepCfg()) {
            case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {

                cout << currentCalibrationStep << endl;
                vector<string> dofsToCalibrate;
                currentCalibrationStep.getDofNames(dofsToCalibrate);
                ParameterSet parameterSet;
                currentCalibrationStep.getParameterSet(parameterSet);
                for (auto p : parameterSet)
                    cout << p << endl;
                ObjectiveFunctionSetup objectiveFunctionSetup = currentCalibrationStep.getObjectiveFunctionSetup();
                MinTorqueError minTorqueError;
                using MySystem = NMSmodelSystem < NMSmodelT, MinTorqueError > ;
                MySystem system(mySubject, trials, minTorqueError, parameterSet, dofsToCalibrate);

                SimulatedAnnealingParameters simanParameters;
                calibrationXmlReader_.getOptimizationAlgorithmParameters(simanParameters);
                Optimizers::SimulatedAnnealing<MySystem> optimizer(system, simanParameters);
                exitflag = optimizer.optimize();

            }   break;

            case CalibrationCfg::TorqueErrorAndSumKneeContactForcesParameterSetDefault: {

                vector<string> dofsToCalibrate;
                currentCalibrationStep.getDofNames(dofsToCalibrate);
                ParameterSet parameterSet;
                currentCalibrationStep.getParameterSet(parameterSet);
                ObjectiveFunctionSetup objectiveFunctionSetup = currentCalibrationStep.getObjectiveFunctionSetup();

                SumKneeContactForces sumKneeContacts(objectiveFunctionSetup);
                sumKneeContacts.setContactModelSetup(contactModelCfg_.getContactModelSetupKneeBalance());
                MinTorqueError minTorqueError;
                MinTorqueErrorAndContactForces minTorqueErrorAndContactForces(minTorqueError, sumKneeContacts);

                using MySystem = NMSmodelSystem < NMSmodelT, MinTorqueErrorAndContactForces > ;
                MySystem system(mySubject, trials, minTorqueErrorAndContactForces, parameterSet, dofsToCalibrate);

                SimulatedAnnealingParameters simanParameters;
                calibrationXmlReader_.getOptimizationAlgorithmParameters(simanParameters);
                Optimizers::SimulatedAnnealing<MySystem> optimizer(system, simanParameters);
                exitflag = optimizer.optimize();
            }   break;

            case CalibrationCfg::TorqueErrorAndSumGenericContactForcesParameterSetDefault: {

                vector<string> dofsToCalibrate;
                currentCalibrationStep.getDofNames(dofsToCalibrate);
                ParameterSet parameterSet;
                currentCalibrationStep.getParameterSet(parameterSet);
                ObjectiveFunctionSetup objectiveFunctionSetup = currentCalibrationStep.getObjectiveFunctionSetup();

                std::shared_ptr<SumGenericContactForces> sumGenericContacts(new SumGenericContactForces(objectiveFunctionSetup));
                sumGenericContacts->setJointContactForcesNames(contactModelCfg_.getContactModelSetupOsim().joints);
                std::shared_ptr<MinTorqueError> minTorqueError(new MinTorqueError());

                ComposedObjectiveFunction objFunction({ minTorqueError, sumGenericContacts });

                using MySystem = NMSmodelSystem < NMSmodelT, ComposedObjectiveFunction >;
                MySystem system(mySubject, trials, objFunction, parameterSet, dofsToCalibrate);

                SimulatedAnnealingParameters simanParameters;
                calibrationXmlReader_.getOptimizationAlgorithmParameters(simanParameters);
                Optimizers::SimulatedAnnealing<MySystem> optimizer(system, simanParameters);
                exitflag = optimizer.optimize();
            }   break;
            } //end switch
        }//end while
        writeXml(mySubject, uncalibratedSubjectFile_, calibrationFile_, calibratedSubjectFile_);
        return exitflag;
    }

    template <typename NMSmodelT>
    bool  CalibrationManager::runHybrid() {
        cout << "Implementation not available in calibration yet. Verify you XML configuration file" << endl;
        return 1;
    }

    bool CalibrationManager::run() {

        bool exitFlag(0);

        NMSModelCfg::RunMode runMode = calibrationXmlReader_.getNMSmodelRunMode();
        switch (runMode) {
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOnline:
            exitFlag = runOpenLoop < NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>>();
            break;

        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline:
            exitFlag = runOpenLoop < NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline>>();
            break;

        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOnline:
            exitFlag = runOpenLoop<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>>();
            break;

        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline:
            exitFlag = runOpenLoop<NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline>>();
            break;

        case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOnline:
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>>();
            break;

        case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOffline:
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline>>();
            break;

        case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonBiSecOnline:
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>>();
            break;

        case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonBiSecOffline:
            exitFlag = runOpenLoop<NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline>>();
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
        }
        return exitFlag;
    }

    template<typename NMSmodelT>
    TrialData CalibrationManager::readTrialData(const string& inputDataFilename, NMSmodelT& mySubject, const string& trialId, const string& emgGeneratorFile) {

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
        //std::cout << "test output Thorben!" << std::endl;
        return trial;

    }

    template<typename NMSmodelT>
    void CalibrationManager::writeXml(NMSmodelT& mySubject,
        const string& uncalibratedSubjectXmlFile,
        const string& calibrationXmlFile,
        const string& newCalibratedSubjectXmlFile) {
        XmlWriter xmlWriter(uncalibratedSubjectXmlFile);
        xmlWriter.setParameters(mySubject);
        xmlWriter.addCalibrationInfo(calibrationXmlFile);
        xmlWriter.setOutputFilename(newCalibratedSubjectXmlFile);
        xmlWriter.write();
    }

    void CalibrationManager::sortMaFilenames(const std::map<std::string,
        std::string>& maMap,
        const std::vector< std::string >& dofNames,
        std::vector< std::string >& maDataFilenames) {

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
