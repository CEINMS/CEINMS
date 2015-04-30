#include "NMSmodelSystem.h"
#include "MinTorqueError.h"
#include "BatchEvaluator.h"
#include "TrialData.h"
#include "SimulatedAnnealing.h"

using CEINMS::TrialData;
#include "InputConnectors.h"
using CEINMS::InputConnectors;
#include "OutputConnectors.h"
using CEINMS::OutputConnectors;
#include "QueuesToTrialData.h"
#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "CeinmsCalibrationSetupXmlReader.h"
#include "CalibrationXmlReader.h"
#include "ExternalTorquesFromStorageFile.h"
#include "InputDataXmlReader.h"
#include "Curve.h"
#include "NMSmodel.h"
#include "Activation/ExponentialActivation.h"
#include "Activation/PiecewiseActivation.h"
//Tendon Policies
#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"
#include "SetupDataStructure.h"
#include <thread>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <list>
using std::list;
#include <chrono>


void setLmtMaFilenames(const string& directory, const vector< string > dofNames, string& lmtDataFilename, vector< string >& maDataFilenames)
{
    std::string pattern{ "_Length.sto" };
    lmtDataFilename = directory + "/" + findFile(pattern, directory);

    int currentDof = 0;
    for (auto& it : dofNames)
    {
        std::string pattern = "_MomentArm_" + it + ".sto";
        std::string maDataFilename = directory + "/" + findFile(pattern, directory);
        maDataFilenames.push_back(maDataFilename);
    }
}

void sortMaFilenames(const std::map<string, string>& maMap, const vector< string > dofNames, vector< string >& maDataFilenames)
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


template<typename NMSmodelT>
CEINMS::TrialData readTrialData(std::string inputDataFilename, NMSmodelT& mySubject, std::string trialId, std::string emgGeneratorFile)
{
    InputDataXmlReader dataLocations(inputDataFilename);
    // CEINMS::InputConnectors* inputConnectors= new CEINMS::InputConnectors();
    //CEINMS::OutputConnectors* outputConnectors = new CEINMS::OutputConnectors();

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

void PrintUsage()
{
    string progName = "CEINMScalibrate";
    cout << "\n\n" << progName << ":\n";// << GetVersionAndDate() << "\n\n";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}


int main(int argc, char** argv) {


    string option = "";
    string setupFileName;
    if (argc < 2) {
        PrintUsage();
        return 0;
    }
    else{
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
                if (argv[i + 1] == 0){
                    cout << "No setup file specified!" << endl;
                    PrintUsage();
                    return -1;
                }
                setupFileName = argv[i + 1];
                break;

                // Print a default setup file
            }
            else if ((option == "-PrintSetup") || (option == "-PS")) {
                if (CeinmsCalibrationSetupXmlReader::writeTemplateCeinmsCalibrationSetupFile("defaultCeinmsCalibrationSetupFile.xml"))
                {
                    std::cout << "Wrote template setup file to defaultCeinmsCalibrationSetupFile.xml" << std::endl;
                    return 0;
                }
                else
                {
                    std::cout << "An error occurred while writing template setup file to defaultCeinmsSetupFIle.xml" << std::endl;
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
    CeinmsCalibrationSetupXmlReader ceinmsSetup(setupFileName);

    string uncalibratedSubjectXmlFile = ceinmsSetup.getSubjectFile();
    string calibrationXmlFile = ceinmsSetup.getCalibrationFile();
    string newCalibratedSubjectXmlFile = ceinmsSetup.getOutputSubjectFile();
    string emgGeneratorFile = ceinmsSetup.getExcitationGeneratorFile();

    //1 read calibration xml
    CalibrationXmlReader calibrationXmlReader(calibrationXmlFile);

    //2 open trial files and put them in a structure
    list<string> calibrationTrialIDs;
    calibrationXmlReader.getCalibrationTrials(calibrationTrialIDs);
    vector<TrialData> trials;

    //3 loop the calibration steps
    NMSModelCfg::RunMode runMode = calibrationXmlReader.getNMSmodelRunMode();
    std::cout << "NMSModelCfg " << runMode << std::endl;
    switch (runMode) {

    case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline: {

        using  MyNMSmodel = NMSmodel < ExponentialActivation, StiffTendon, CurveMode::Offline >;
        MyNMSmodel mySubject;
        setupSubject(mySubject, uncalibratedSubjectXmlFile);
        for (list<string>::iterator trialIt = calibrationTrialIDs.begin(); trialIt != calibrationTrialIDs.end(); ++trialIt)
            trials.push_back(readTrialData(*trialIt, mySubject, *trialIt, emgGeneratorFile));
        CalibrationStep currentCalibrationStep;
        while (calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
            std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
            switch (currentCalibrationStep.getStepCfg()) {
            case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
              
                vector<string> dofsToCalibrate;
                currentCalibrationStep.getDofNames(dofsToCalibrate);
                ParameterSet parameterSet;
                currentCalibrationStep.getParameterSet(parameterSet);
                using MySystem = CEINMS::NMSmodelSystem < MyNMSmodel, CEINMS::MinTorqueError > ;
                MySystem system(mySubject, trials, CEINMS::MinTorqueError(), parameterSet, dofsToCalibrate);

                SimulatedAnnealingParameters simanParameters;
                calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                CEINMS::Optimizers::SimulatedAnnealing<MySystem> optimizer(system, simanParameters);

                auto timeBegin = std::chrono::high_resolution_clock::now();
                optimizer.optimize();
                auto timeEnd = std::chrono::high_resolution_clock::now();
                std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin).count() << "ms" << std::endl;
            }
                break;
            default:
                std::cerr << "Calibration Step not valid\n";
            }
        } //end while


    } //end case
        break;



    case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline: {

        using  MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline >;
        MyNMSmodel mySubject;
        setupSubject(mySubject, uncalibratedSubjectXmlFile);
        for (list<string>::iterator trialIt = calibrationTrialIDs.begin(); trialIt != calibrationTrialIDs.end(); ++trialIt)
            trials.push_back(readTrialData(*trialIt, mySubject, *trialIt, emgGeneratorFile));
        CalibrationStep currentCalibrationStep;
        while (calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
            std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
            switch (currentCalibrationStep.getStepCfg()) {
            case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
                CEINMS::MinTorqueError objFun;
                vector<string> dofsToCalibrate;
                currentCalibrationStep.getDofNames(dofsToCalibrate);
                ParameterSet parameterSet;
                currentCalibrationStep.getParameterSet(parameterSet);
                using MySystem = CEINMS::NMSmodelSystem < MyNMSmodel, CEINMS::MinTorqueError >;
                MySystem system(mySubject, trials, CEINMS::MinTorqueError(), parameterSet, dofsToCalibrate);

                SimulatedAnnealingParameters simanParameters;
                calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                CEINMS::Optimizers::SimulatedAnnealing<MySystem> optimizer(system, simanParameters);

                auto timeBegin = std::chrono::high_resolution_clock::now();
                optimizer.optimize();
                auto timeEnd = std::chrono::high_resolution_clock::now();
                std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin).count() << "ms" << std::endl;
            }
                break;
            default:
                std::cerr << "Calibration Step not valid\n";
            }
        } //end while


    } //end case
        break;

    }
    return 0;
}