//__________________________________________________________________________
// Author: Claudio Pizzolato, September 2013
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
//Tendon Policies
#include "Tendon/StiffTendon.h"
//Computation Modes
#include "TorquesComputation.h"
#include "ComputationMode_Default.h"
#include "ComputationMode_Fast.h"
//Calibration Parameters
#include "StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.h"
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




template <typename T>
void setupSubject(T& mySubject, string configurationFile) {
    
    SetupDataStructure<T> setupData(configurationFile);
    setupData.createCurves();
    setupData.createMuscles(mySubject);
    setupData.createDoFs(mySubject);
    
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


int main(int argc, char** argv){
    
    
    
//    string calibrationXmlFile("calibration.xml");
//    string uncalibratedSubjectXmlFile("uncalibratedSubject.xml");
//    string newCalibratedSubjectXmlFile("calibratedSubject.xml");

    string calibrationXmlFile(argv[1]);
    string uncalibratedSubjectXmlFile(argv[2]);
    string newCalibratedSubjectXmlFile(argv[3]);

    printHeader();
    printAuthors();
    
    try {
        std::auto_ptr<SubjectType> subjectPointer (subject (uncalibratedSubjectXmlFile));
    }  
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
    
    //1 read calibration xml
    CalibrationXmlReader calibrationXmlReader(calibrationXmlFile);
    
    //2 open trial files and put them in a structure
    list<string> calibrationTrialIDs;
    calibrationXmlReader.getCalibrationTrials(calibrationTrialIDs);
    vector<TrialData> trials;  
    string trialsInputDirectory = "/home/co/Dropbox/Ricerca/versioning/ceinms/TestData/unix/"; //TODO get it from xml
        
    
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
                        typedef SimulatedAnnealing<MyParameters, MyObjFunction, MyComputationMode, MyNMSmodel> MyOptimizator;
                        
                        vector<string> dofsToCalibrate;
                        currentCalibrationStep.getDofNames(dofsToCalibrate);
                        MyComputationMode torqueComputation(mySubject, trials, dofsToCalibrate);
                        SimulatedAnnealingParameters simanParameters;
                        calibrationXmlReader.getOptimizationAlgorithmParameters(simanParameters);
                        MyOptimizator optimizator(mySubject, dofsToCalibrate, torqueComputation, simanParameters);
                        optimizator.optimize();
                    }
                    break;

                    default:
                        std::cerr << "Calibration Step not valid\n";
                }       
                
            }
            XmlWriter xmlWriter(uncalibratedSubjectXmlFile);
            xmlWriter.setParameters(mySubject);
            xmlWriter.addCalibrationInfo(calibrationXmlFile);
            xmlWriter.setOutputFilename(newCalibratedSubjectXmlFile);
            xmlWriter.write();
              
            
       
        }
        break;
        default:
            std::cerr << "Model configuration not valid\n";
                        
     }
 
  
    
    exit(EXIT_SUCCESS);
}

























