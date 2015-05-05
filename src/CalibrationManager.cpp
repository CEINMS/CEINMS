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
 * Author(s):                                                                 *
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

#include "NMSmodel.h"

#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"
#include "Activation/ExponentialActivation.h"
#include "Activation/PiecewiseActivation.h"
#include "MinTorqueError.h"
#include "SimulatedAnnealing.h"
#include "NMSmodelSystem.h"
#include "Curve.h"
#include "InputDataXmlReader.h"
#include "EMGFromFile.h"
#include "ExternalTorquesFromStorageFile.h"
#include "LmtMaFromStorageFile.h"

#include "QueuesToTrialData.h"
#include "InputConnectors.h"
#include "OutputConnectors.h"

#include "XmlWriter.h"
#include "FileUtils.h"

namespace ceinms {


    CalibrationManager::CalibrationManager(const string& setupFilename) :
        ceinmsCalibrationSetup_(setupFilename),
        uncalibratedSubjectFile_(ceinmsCalibrationSetup_.getSubjectFile()),
        calibrationFile_(ceinmsCalibrationSetup_.getCalibrationFile()),
        calibrationXmlReader_(ceinmsCalibrationSetup_.getCalibrationFile()),
        calibratedSubjectFile_(ceinmsCalibrationSetup_.getOutputSubjectFile()),
        emgGeneratorFile_(ceinmsCalibrationSetup_.getExcitationGeneratorFile()) {

        calibrationXmlReader_.getCalibrationTrials(calibrationTrialIDs_);
    }

    template <typename NMSmodelT>
    bool CalibrationManager::runOpenLoop() {
        bool exitflag(false);
        NMSmodelT mySubject;
        setupSubject(mySubject, uncalibratedSubjectFile_, calibrationXmlReader_.getTolerance());

        vector<TrialData> trials;
        for (list<string>::iterator trialIt = calibrationTrialIDs_.begin(); trialIt != calibrationTrialIDs_.end(); ++trialIt)
            trials.push_back(readTrialData(*trialIt, mySubject, *trialIt, emgGeneratorFile_));

        CalibrationStep currentCalibrationStep;
        while (calibrationXmlReader_.popNextCalibrationStep(currentCalibrationStep)) {
            std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
            switch (currentCalibrationStep.getStepCfg()) {
                case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {

                    vector<string> dofsToCalibrate;
                    currentCalibrationStep.getDofNames(dofsToCalibrate);
                    ParameterSet parameterSet;
                    currentCalibrationStep.getParameterSet(parameterSet);

                    using MySystem = NMSmodelSystem < NMSmodelT, MinTorqueError>;
                    MySystem system(mySubject, trials, MinTorqueError(), parameterSet, dofsToCalibrate);

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

        std::unique_ptr<InputConnectors> inputConnectors(new InputConnectors);
        std::unique_ptr<OutputConnectors> outputConnectors(new OutputConnectors);

        string emgFilename(dataLocations.getExcitationsFile());
        EMGFromFile emgProducer(*inputConnectors, mySubject, emgFilename, emgGeneratorFile);

        vector< string > dofNames;
        mySubject.getDoFNames(dofNames);
        vector< string > maFilename;
        sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
        LmtMaFromStorageFile lmtMaProducer(*inputConnectors, mySubject, dataLocations.getLmtFile(), maFilename);

        string externalTorqueFilename(dataLocations.getExternalTorqueFile());
        ExternalTorquesFromStorageFile externalTorquesProducer(*inputConnectors, mySubject, externalTorqueFilename);

        QueuesToTrialData queuesToTrialData(*inputConnectors, *outputConnectors, mySubject, trialId);

        inputConnectors->doneWithSubscription.setCount(4);
        outputConnectors->doneWithExecution.setCount(1);

        // 4. start the threads
        std::thread emgProdThread(std::ref(emgProducer));
        std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        std::thread queuesToTrialDataThread(std::ref(queuesToTrialData));

        emgProdThread.join();
        lmtMaProdThread.join();
        externalTorquesProdThread.join();
        queuesToTrialDataThread.join();

        if (queuesToTrialData.getTrialData().lmtData.getNRows() < 1)
        {
            std::cout << "ERROR: No data was read for trial " << trialId << std::endl;
            exit(EXIT_FAILURE);
        }
        return queuesToTrialData.getTrialData();
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
