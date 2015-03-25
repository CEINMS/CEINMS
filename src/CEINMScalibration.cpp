//__________________________________________________________________________
// Author: Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "CalibrationXmlReader.h"
#include "InputDataXmlReader.h"
#include "subject.hxx"
#include "NMSmodel.h"
#include "SetupDataStructure.h"
//Activation Policies
#include "Activation/ExponentialActivation.h"
#include "Activation/PiecewiseActivation.h"
//Tendon Policies
#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"
//Computation Modes
#include "TorquesComputation.h"
#include "ComputationMode_Default.h"
//Calibration Parameters
#include "StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.h"
#include "ParametersFromXml.h"
//Objective Functions
#include "SumMinObjectiveFunction_singleF.h"
//Optimization Algorithms
#include "SimulatedAnnealing.h"

#include "XmlWriter.h"
#include "FileUtils.h"

#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "ExternalTorquesFromStorageFile.h"
#include "QueuesToTrialData.h"
using namespace CEINMS;
#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <list>
using std::list;
#include <map>

#include <stdlib.h>

#include <ctime>
#include <chrono>

#include <CeinmsCalibrationSetupXmlReader.h>


template <typename T>
void setupSubject(T& mySubject, string configurationFile) {
    
    SetupDataStructure<T> setupData(configurationFile);
    setupData.createCurves();
    setupData.createMuscles(mySubject);
    setupData.createDoFs(mySubject);
    
}
     
     
template<typename T>
void writeXml(T& mySubject, string& uncalibratedSubjectXmlFile, string& calibrationXmlFile, string newCalibratedSubjectXmlFile) {
    XmlWriter xmlWriter(uncalibratedSubjectXmlFile);
    xmlWriter.setParameters(mySubject);
    xmlWriter.addCalibrationInfo(calibrationXmlFile);
    xmlWriter.setOutputFilename(newCalibratedSubjectXmlFile);
    xmlWriter.write();
}
    
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
         << "+-+-+-+-+-+-+-+\n\n"
         << "Calibration\n\n";                   
        
    }
    
void printAuthors() {
    
    time_t now = time(0);
    tm *gmtm = gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year+1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, Massimo Sartori, David Lloyd\n\n";
    
    cout << "Software developers: Claudio Pizzolato, Monica Reggiani\n";
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

template<typename NMSmodel>
TrialData readTrialData(std::string inputDataFilename, NMSmodel& mySubject, std::string trialId, std::string emgGeneratorFile)
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
    ExternalTorquesFromStorageFile externalTorquesProducer(*inputConnectors,  mySubject, externalTorqueFilename);

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

    if (queuesToTrialData.getTrialData().noLmtSteps_ < 1)
    {
        std::cout << "ERROR: No data was read for trial " << trialId << std::endl;
        exit(EXIT_FAILURE);
    }
    return queuesToTrialData.getTrialData();
};




int main(int argc, char** argv){
    
    
    
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
     switch(runMode) {
       
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline: {
            
            typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, uncalibratedSubjectXmlFile);
            for (list<string>::iterator trialIt = calibrationTrialIDs.begin(); trialIt != calibrationTrialIDs.end(); ++trialIt)
            {
                trials.push_back(readTrialData(*trialIt , mySubject, *trialIt, emgGeneratorFile));
            }
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef ParametersFromXml<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        ParameterSet parameterSet;
                        currentCalibrationStep.getParameterSet(parameterSet);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        auto timeBegin = std::chrono::high_resolution_clock::now();
                        optimizator.optimize();
                        auto timeEnd = std::chrono::high_resolution_clock::now();
                        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin).count() << "ms" << std::endl;
                    }
                    break;
                    default:
                        std::cerr << "Calibration Step not valid\n";
                }      
            } //end while
      
            writeXml(mySubject, uncalibratedSubjectXmlFile, calibrationXmlFile, newCalibratedSubjectXmlFile);
        } //end case
        break;
                
        case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOffline: {
            typedef NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, uncalibratedSubjectXmlFile);
            for (list<string>::iterator trialIt = calibrationTrialIDs.begin(); trialIt != calibrationTrialIDs.end(); ++trialIt)
            {
                trials.push_back(readTrialData(*trialIt, mySubject, *trialIt, emgGeneratorFile));
            }
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef ParametersFromXml<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        ParameterSet parameterSet;
                        currentCalibrationStep.getParameterSet(parameterSet);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;
                    
                    default:
                        std::cerr << "Calibration Step not valid\n";
                } //       
                
            } 
            
            writeXml(mySubject, uncalibratedSubjectXmlFile, calibrationXmlFile, newCalibratedSubjectXmlFile);
              
        } //end case
        break;
        
        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline: {
            typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, uncalibratedSubjectXmlFile);
            for (list<string>::iterator trialIt = calibrationTrialIDs.begin(); trialIt != calibrationTrialIDs.end(); ++trialIt)
            {
                trials.push_back(readTrialData(*trialIt, mySubject, *trialIt, emgGeneratorFile));
            }
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef ParametersFromXml<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        ParameterSet parameterSet;
                        currentCalibrationStep.getParameterSet(parameterSet);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;
                    
                    default:
                        std::cerr << "Calibration Step not valid\n";
                } //       
                
            } 
            
            writeXml(mySubject, uncalibratedSubjectXmlFile, calibrationXmlFile, newCalibratedSubjectXmlFile);
              
        } //end case
        break;
        
        case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonBiSecOffline: {
            typedef NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, uncalibratedSubjectXmlFile);
            //vector<string> muscleNames, dofNames;
            //mySubject.getMuscleNames(muscleNames);
            //mySubject.getDoFNames(dofNames);
            //InputDataInterpreter inputData(muscleNames, dofNames);
            //inputData.setInputDirectory(trialsInputDirectory);
            //inputData.convert(calibrationTrialIDs, trials);
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef ParametersFromXml<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        ParameterSet parameterSet;
                        currentCalibrationStep.getParameterSet(parameterSet);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;
                    
                    default:
                        std::cerr << "Calibration Step not valid\n";
                } //       
                
            } 
            
            writeXml(mySubject, uncalibratedSubjectXmlFile, calibrationXmlFile, newCalibratedSubjectXmlFile);
              
        } //end case
        break;

        default:
            std::cerr << "Model configuration not valid\n";
                        
     }
 
  
    
    exit(EXIT_SUCCESS);
}

























