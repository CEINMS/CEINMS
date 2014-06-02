//__________________________________________________________________________
// Author: Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "CalibrationXmlReader.h"
#include "subject.hxx"
#include "NMSmodel.h"
#include "InputDataInterpreter.h"
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
#include "ComputationMode_Fast.h"
//Calibration Parameters
#include "StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.h"
#include "ParametersFromXml.h"
//Objective Functions
#include "SumMinObjectiveFunction_singleF.h"
//Optimization Algorithms
#include "SimulatedAnnealing.h"

#include "XmlWriter.h"

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <list>
using std::list;

#include <stdlib.h>

#include <ctime>

#include <boost/program_options.hpp>
namespace po = boost::program_options;



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


int main(int ac, char** av){
    
    
    
//    string calibrationXmlFile("calibration.xml");
//    string uncalibratedSubjectXmlFile("uncalibratedSubject.xml");
//    string newCalibratedSubjectXmlFile("calibratedSubject.xml");

    string calibrationXmlFile;
    string uncalibratedSubjectXmlFile;
    string newCalibratedSubjectXmlFile;
    printHeader();
    printAuthors();

    int opt;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("calibration,c", po::value<string>(&calibrationXmlFile),  "calibration xml file")
    ("subject,s", po::value<string>(&uncalibratedSubjectXmlFile), "subject xml file that cointains the initial parameter values")
    ("output,o", po::value<string>(&newCalibratedSubjectXmlFile)->default_value("calibratedSubject.xml"), "name of the calibrated subject file");
    
    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);    

    if (vm.count("help") || ac < 5) {
        cout << desc << "\n";
        return 1;
    }

    
//     try {
//         std::auto_ptr<SubjectType> subjectPointer (subject (uncalibratedSubjectXmlFile));
//     }
//     catch (const xml_schema::exception& e) {
//         cout << e << endl;
//         exit(EXIT_FAILURE);
//     }
    
    //1 read calibration xml
    CalibrationXmlReader calibrationXmlReader(calibrationXmlFile);
    
    //2 open trial files and put them in a structure
    list<string> calibrationTrialIDs;
    calibrationXmlReader.getCalibrationTrials(calibrationTrialIDs);
    vector<TrialData> trials;  
    string trialsInputDirectory;
	calibrationXmlReader.getTrialsDirectory(trialsInputDirectory); //TODO get it from xml
        
    
    //3 loop the calibration steps
    NMSModelCfg::RunMode runMode = calibrationXmlReader.getNMSmodelRunMode();
    std::cout << "NMSModelCfg " << runMode << std::endl;
     switch(runMode) {
       
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline: {
            
            typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, uncalibratedSubjectXmlFile);
            vector<string> muscleNames, dofNames;
            mySubject.getMuscleNames(muscleNames);
            mySubject.getDoFNames(dofNames);
            InputDataInterpreter inputData(muscleNames, dofNames);
            inputData.setInputDirectory(trialsInputDirectory);
            inputData.convert(calibrationTrialIDs, trials);
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;
                    
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;

                    case CalibrationCfg::MinimizeTorqueErrorParameterSetFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
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
                }      
            } //end while
      
            writeXml(mySubject, uncalibratedSubjectXmlFile, calibrationXmlFile, newCalibratedSubjectXmlFile);
        } //end case
        break;
                
        case NMSModelCfg::OpenLoopPiecewiseActivationStiffTendonOffline: {
            typedef NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, uncalibratedSubjectXmlFile);
            vector<string> muscleNames, dofNames;
            mySubject.getMuscleNames(muscleNames);
            mySubject.getDoFNames(dofNames);
            InputDataInterpreter inputData(muscleNames, dofNames);
            inputData.setInputDirectory(trialsInputDirectory);
            inputData.convert(calibrationTrialIDs, trials);
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;
                    
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;

                    case CalibrationCfg::MinimizeTorqueErrorParameterSetFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
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
            vector<string> muscleNames, dofNames;
            mySubject.getMuscleNames(muscleNames);
            mySubject.getDoFNames(dofNames);
            InputDataInterpreter inputData(muscleNames, dofNames);
            inputData.setInputDirectory(trialsInputDirectory);
            inputData.convert(calibrationTrialIDs, trials);
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;
                    
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;

                    case CalibrationCfg::MinimizeTorqueErrorParameterSetFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
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
            vector<string> muscleNames, dofNames;
            mySubject.getMuscleNames(muscleNames);
            mySubject.getDoFNames(dofNames);
            InputDataInterpreter inputData(muscleNames, dofNames);
            inputData.setInputDirectory(trialsInputDirectory);
            inputData.convert(calibrationTrialIDs, trials);
            CalibrationStep currentCalibrationStep;
            while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch(currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;
                    
                    case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleDefault: {
                        typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
                        typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
                        typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyParameters parameterPolicy(mySubject, dofsToCalibrate);
                        MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;

                    case CalibrationCfg::MinimizeTorqueErrorParameterSetFast: {
                        typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
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
        
//         case NMSModelCfg::OpenLoopExponentialActivationElasticTendonOffline: {
//             typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline> MyNMSmodel;
//             MyNMSmodel mySubject;
//             setupSubject(mySubject, uncalibratedSubjectXmlFile);
//             vector<string> muscleNames, dofNames;
//             mySubject.getMuscleNames(muscleNames);
//             mySubject.getDoFNames(dofNames);
//             InputDataInterpreter inputData(muscleNames, dofNames);
//             inputData.setInputDirectory(trialsInputDirectory);
//             inputData.convert(calibrationTrialIDs, trials);
//             CalibrationStep currentCalibrationStep;
//             while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
//                 std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
//                 switch(currentCalibrationStep.getStepCfg()) {
//                     case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleFast: {
//                         typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     break;
//                     
//                     case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleDefault: {
//                         typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     break;
// 
//                     case CalibrationCfg::MinimizeTorqueErrorParameterSetFast: {
//                         typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef ParametersFromXml<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         ParameterSet parameterSet;
//                         currentCalibrationStep.getParameterSet(parameterSet);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     
//                     case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
//                         typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef ParametersFromXml<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         ParameterSet parameterSet;
//                         currentCalibrationStep.getParameterSet(parameterSet);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     break;
//                     
//                     default:
//                         std::cerr << "Calibration Step not valid\n";
//                 } //       
//                 
//             } 
//             
//             writeXml(mySubject, uncalibratedSubjectXmlFile, calibrationXmlFile, newCalibratedSubjectXmlFile);
//               
//         } //end case
//         break;
//         
//         case NMSModelCfg::OpenLoopPiecewiseActivationElasticTendonOffline: {
//             typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline> MyNMSmodel;
//             MyNMSmodel mySubject;
//             setupSubject(mySubject, uncalibratedSubjectXmlFile);
//             vector<string> muscleNames, dofNames;
//             mySubject.getMuscleNames(muscleNames);
//             mySubject.getDoFNames(dofNames);
//             InputDataInterpreter inputData(muscleNames, dofNames);
//             inputData.setInputDirectory(trialsInputDirectory);
//             inputData.convert(calibrationTrialIDs, trials);
//             CalibrationStep currentCalibrationStep;
//             while(calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
//                 std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
//                 switch(currentCalibrationStep.getStepCfg()) {
//                     case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleFast: {
//                         typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     break;
//                     
//                     case CalibrationCfg::MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleDefault: {
//                         typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     break;
// 
//                     case CalibrationCfg::MinimizeTorqueErrorParameterSetFast: {
//                         typedef TorquesComputation<ComputationMode_Fast<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef ParametersFromXml<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         ParameterSet parameterSet;
//                         currentCalibrationStep.getParameterSet(parameterSet);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     
//                     case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
//                         typedef TorquesComputation<ComputationMode_Default<MyNMSmodel>, MyNMSmodel> MyComputationMode;
//                         typedef ParametersFromXml<MyNMSmodel> MyParameters;
//                         typedef SumMinObjectiveFunction_singleF<MyComputationMode> MyObjFunction;            
//                         typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode> MyOptimizator;
//                         
//                         vector<string> dofsToCalibrate;
//                         currentCalibrationStep.getDofNames(dofsToCalibrate);
//                         MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
//                         SimulatedAnnealingParameters simanParameters;
//                         calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
//                         ParameterSet parameterSet;
//                         currentCalibrationStep.getParameterSet(parameterSet);
//                         MyParameters parameterPolicy(mySubject, dofsToCalibrate, parameterSet);
//                         MyOptimizator optimizator(parameterPolicy, torqueComputation, simanParameters);
//                         optimizator.optimize();
//                     }
//                     break;
//                     
//                     default:
//                         std::cerr << "Calibration Step not valid\n";
//                 } //       
//                 
//             } 
//             
//             writeXml(mySubject, uncalibratedSubjectXmlFile, calibrationXmlFile, newCalibratedSubjectXmlFile);
//               
//         } //end case
//         break;
        
        default:
            std::cerr << "Model configuration not valid\n";
                        
     }
 
  
    
    exit(EXIT_SUCCESS);
}

























