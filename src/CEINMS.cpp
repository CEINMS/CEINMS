//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"

#include "SetupDataStructure.h"
#include "Activation/ExponentialActivation.h"
#include "ErrorMinimizerAnnealing.h"
#include "HybridWeightings.h"
#include "Curve.h"
#include "ExecutionXmlReader.h"
#include "InputDataXmlReader.h"
#include "FileUtils.h"

#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "ExternalTorquesFromStorageFile.h"
#include "ModelEvaluationOnline.h" 

#include "InputQueues.h"

#include "LoggerOnQueues.h"
#include "QueuesToStorageFiles.h"

#include "ModelEvaluationOffline.h"
#include "ModelEvaluationHybrid.h"

#include <ctime>

#include <iomanip>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <stdlib.h>

#include <CeinmsSetupXmlReader.h>


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
         << "+-+-+-+-+-+-+-+\n\n";                   
        
    }
    
void printAuthors() {
    
    time_t now = std::time(0);
    tm *gmtm = std::gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year+1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, David Lloyd, Massimo Sartori\n\n";
    
    cout << "Software developers: Claudio Pizzolato, Elena Ceseracciu, Monica Reggiani\n";
}

void PrintUsage()
{
    string progName= "CEINMS";
    cout << "\n\n" << progName << ":\n";// << GetVersionAndDate() << "\n\n";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}

void setLmtMaFilenames(const string& inputDirectory, const vector< string > dofNames, string& lmtDataFilename, vector< string >& maDataFilenames)
{
  std::string pattern{"_MuscleAnalysis_Length.sto"};
  std::string directory{inputDirectory + "/MuscleAnalysis/"};
  lmtDataFilename = directory + findFile(pattern, directory);
  
  int currentDof = 0; 
  for (auto& it: dofNames)
  {
    std::string pattern = "_MomentArm_" + it + ".sto";
    std::string maDataFilename = directory + findFile(pattern, directory);
    maDataFilenames.push_back(maDataFilename); 
  }
}


void sortMaFilenames(const map<string, string>& maMap, const vector< string > dofNames, vector< string >& maDataFilenames)
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



void consumeAndStore(CEINMS::Concurrency::Queue< std::vector<double> >& queue, const string& outputFileName, const vector<string>& header) {
  
  queue.subscribe(); 
  CEINMS::InputConnectors::doneWithSubscription.wait();
  
  std::ofstream outputFile (outputFileName);
  if (!outputFile.is_open())
  { 
    std::cout << "Problem opening file" << outputFileName << std::endl;  
    exit(EXIT_FAILURE); 
  }
    
  
  for (auto it: header)
    outputFile << it << ",";
  outputFile << "Time" << ",\n";
  
  for(;;) {
     vector<double> item = queue.pop(); 
     for (auto it : item)
       outputFile << std::setprecision(8)  << it << ","; 
     outputFile << endl; 
  }
}

int main(int argc, char** argv) {
 
    printHeader();
    printAuthors();
    
#ifdef LOG  
  cout << "Check configuration data...\n";
#endif

    string option="";
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
                if (CeinmsSetupXmlReader::writeTemplateCeinmsSetupFile("defaultCeinmsSetupFile.xml"))
                {
                    std::cout << "Wrote template setup file to defaultCeinmsSetupFile.xml" << std::endl;
                    return 0;
                }
                else
                {
                    std::cout << "An error occurred while writing template setup file to defaultCeinmsSetupFile.xml" << std::endl;
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
    CeinmsSetupXmlReader ceinmsSetup(setupFileName);

    string subjectFile = ceinmsSetup.getSubjectFile();
    string executionFile = ceinmsSetup.getExecutionFile();
    string inputData = ceinmsSetup.getInputDataFile();
    string outputDirectory = ceinmsSetup.getOutputDirectory();
    string emgGeneratorFile = ceinmsSetup.getEmgGeneratorFile();


    ExecutionXmlReader executionCfg(executionFile);             
    InputDataXmlReader dataLocations(inputData);

    NMSModelCfg::RunMode runMode = executionCfg.getRunMode();
   
//    NMSModelCfg::RunMode runMode = NMSModelCfg::HybridPiecewiseActivationElasticTendonOnline;
    switch(runMode) {
               
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOnline: {
          
            // 1. define the model
            typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, subjectFile);
          
            // 2. define the thread connecting with the input sources          
            string emgFilename(dataLocations.getEmgFile());
            EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);
          
            vector< string > dofNames; 
            mySubject.getDoFNames(dofNames);
            vector< string > maFilename;
            sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
            LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);
                   
            string externalTorqueFilename(dataLocations.getExternalTorqueFile());
            ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename); 
               
            // 2b. define the thread consuming the output sources
            vector<string> valuesToWrite = {"Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques"};
            QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory) ;  
          
            // 3. define the model simulator
            vector<string> valuesToLog = {"Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques"};
            ModelEvaluationOnline<MyNMSmodel, LoggerOnQueues> simulator(mySubject, valuesToLog);
     
            CEINMS::InputConnectors::doneWithSubscription.setCount(5);
            CEINMS::OutputConnectors::doneWithExecution.setCount(2);
          
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

            break;
        }
       
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline: {

            // 1. define the model
            typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, subjectFile);

            // 2. define the thread connecting with the input sources          
            string emgFilename(dataLocations.getEmgFile());
            EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);

            vector< string > dofNames;
            mySubject.getDoFNames(dofNames);
            vector< string > maFilename;
            sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
            LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);

            string externalTorqueFilename(dataLocations.getExternalTorqueFile());
            ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);

            // 2b. define the thread consuming the output sources
            vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
            QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory);

            // 3. define the model simulator
            vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
            ModelEvaluationOffline<MyNMSmodel, LoggerOnQueues> simulator(mySubject, valuesToLog);

            CEINMS::InputConnectors::doneWithSubscription.setCount(5);
            CEINMS::OutputConnectors::doneWithExecution.setCount(2);

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

            break;
        }
        //
        //case NMSModelCfg::OpenLoopExponentialActivationElasticTendonOnline: {

        //    // 1. define the model
        //    typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Online>, CurveMode::Online> MyNMSmodel;
        //    MyNMSmodel mySubject;
        //    setupSubject(mySubject, subjectFile);

        //    // 2. define the thread connecting with the input sources          
        //    string emgFilename(dataLocations.getEmgFile());
        //    EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);

        //    vector< string > dofNames;
        //    mySubject.getDoFNames(dofNames);
        //    vector< string > maFilename;
        //    sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
        //    LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);

        //    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
        //    ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);

        //    // 2b. define the thread consuming the output sources
        //    vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
        //    QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory);

        //    // 3. define the model simulator
        //    vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
        //    ModelEvaluationOnline<MyNMSmodel, LoggerOnQueues> simulator(mySubject, valuesToLog);

        //    CEINMS::InputConnectors::doneWithSubscription.setCount(5);
        //    CEINMS::OutputConnectors::doneWithExecution.setCount(2);

        //    // 4. start the threads
        //    std::thread emgProdThread(std::ref(emgProducer));
        //    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        //    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        //    std::thread simulatorThread(std::ref(simulator));
        //    std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));

        //    emgProdThread.join();
        //    lmtMaProdThread.join();
        //    externalTorquesProdThread.join();
        //    simulatorThread.join();
        //    queuesToStorageFilesThread.join();
        //    break;

        //}
        //     
        // 
        // case NMSModelCfg::OpenLoopExponentialActivationElasticTendonOffline: {

        //     // 1. define the model
        //     typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline> MyNMSmodel;
        //     MyNMSmodel mySubject;
        //     setupSubject(mySubject, subjectFile);

        //     // 2. define the thread connecting with the input sources          
        //     string emgFilename(dataLocations.getEmgFile());
        //     EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);

        //     vector< string > dofNames;
        //     mySubject.getDoFNames(dofNames);
        //     vector< string > maFilename;
        //     sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
        //     LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);

        //     string externalTorqueFilename(dataLocations.getExternalTorqueFile());
        //     ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);

        //     // 2b. define the thread consuming the output sources
        //     vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
        //     QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory);

        //     // 3. define the model simulator
        //     vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
        //     ModelEvaluationOffline<MyNMSmodel, LoggerOnQueues> simulator(mySubject, valuesToLog);

        //     CEINMS::InputConnectors::doneWithSubscription.setCount(5);
        //     CEINMS::OutputConnectors::doneWithExecution.setCount(2);

        //     // 4. start the threads
        //     std::thread emgProdThread(std::ref(emgProducer));
        //     std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
        //     std::thread lmtMaProdThread(std::ref(lmtMaProducer));
        //     std::thread simulatorThread(std::ref(simulator));
        //     std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));

        //     emgProdThread.join();
        //     lmtMaProdThread.join();
        //     externalTorquesProdThread.join();
        //     simulatorThread.join();
        //     queuesToStorageFilesThread.join();
        //     break;
        // }
        //
         case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOnline: {
             // 1. define the model
             typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> MyNMSmodel;
             MyNMSmodel mySubject;
             setupSubject(mySubject, subjectFile);

             // 2. define the thread connecting with the input sources          
             string emgFilename(dataLocations.getEmgFile());
             EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);

             vector< string > dofNames;
             mySubject.getDoFNames(dofNames);
             vector< string > maFilename;
             sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
             LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);

             string externalTorqueFilename(dataLocations.getExternalTorqueFile());
             ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);

             // 2b. define the thread consuming the output sources
             vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
             QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory);

             // 3. define the model simulator
             vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
             ModelEvaluationOnline<MyNMSmodel, LoggerOnQueues> simulator(mySubject, valuesToLog);

             CEINMS::InputConnectors::doneWithSubscription.setCount(5);
             CEINMS::OutputConnectors::doneWithExecution.setCount(2);

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

             break;
         }
         
         case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline: {
             // 1. define the model
             typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
             MyNMSmodel mySubject;
             setupSubject(mySubject, subjectFile);

             // 2. define the thread connecting with the input sources          
             string emgFilename(dataLocations.getEmgFile());
             EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);

             vector< string > dofNames;
             mySubject.getDoFNames(dofNames);
             vector< string > maFilename;
             sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
             LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);

             string externalTorqueFilename(dataLocations.getExternalTorqueFile());
             ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);

             // 2b. define the thread consuming the output sources
             vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
             QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory);

             // 3. define the model simulator
             vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques" };
             ModelEvaluationOffline<MyNMSmodel, LoggerOnQueues> simulator(mySubject, valuesToLog);

             CEINMS::InputConnectors::doneWithSubscription.setCount(5);
             CEINMS::OutputConnectors::doneWithExecution.setCount(2);

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

             break;

         }
         
         
         case NMSModelCfg::HybridExponentialActivationStiffTendonOnline: { 

             // 1. define the model
             typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online> MyNMSmodel;
             MyNMSmodel mySubject;
             setupSubject(mySubject, subjectFile);

             // 2. define the thread connecting with the input sources          
             string emgFilename(dataLocations.getEmgFile());
             EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);

             vector< string > dofNames;
             mySubject.getDoFNames(dofNames);
             vector< string > maFilename;
             sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
             LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);

             string externalTorqueFilename(dataLocations.getExternalTorqueFile());
             ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);

             // 2b. define the thread consuming the output sources
             vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques", "AdjustedEmgs" };
             QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory);

             // 3. define the model simulator
             vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques", "AdjustedEmgs" };
            
             CEINMS::InputConnectors::doneWithSubscription.setCount(5);
             CEINMS::OutputConnectors::doneWithExecution.setCount(2);

             // 4. define the optimiser
             typedef Hybrid::ErrorMinimizerAnnealing<MyNMSmodel> MyErrorMinimizer;
             MyErrorMinimizer errorMinimizer(mySubject);
             HybridWeightings weightings;
             executionCfg.getHybridWeightings(weightings.alpha, weightings.beta, weightings.gamma);
             errorMinimizer.setWeightings(weightings);
             vector<string> toPredict, toTrack;
             executionCfg.getMusclesToPredict(toPredict);
             executionCfg.getMusclesToTrack(toTrack);
             errorMinimizer.setMusclesNamesWithEmgToPredict(toPredict);
             errorMinimizer.setMusclesNamesWithEmgToTrack(toTrack);
             double rt, t, epsilon;
             unsigned noEpsilon, ns, nt, maxNoEval;
             executionCfg.getAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
             errorMinimizer.setAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
             
             ModelEvaluationHybrid<MyNMSmodel, MyErrorMinimizer, LoggerOnQueues> simulator(mySubject, errorMinimizer, valuesToLog);
             
             
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
             break;
         } 
         
           case NMSModelCfg::HybridExponentialActivationElasticTendonBiSecOnline: { 
               // 1. define the model
               typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> MyNMSmodel;
               MyNMSmodel mySubject;
               setupSubject(mySubject, subjectFile);

               // 2. define the thread connecting with the input sources          
               string emgFilename(dataLocations.getEmgFile());
               EMGFromFile emgProducer(mySubject, emgFilename, emgGeneratorFile);

               vector< string > dofNames;
               mySubject.getDoFNames(dofNames);
               vector< string > maFilename;
               sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
               LmtMaFromStorageFile lmtMaProducer(mySubject, dataLocations.getLmtFile(), maFilename);

               string externalTorqueFilename(dataLocations.getExternalTorqueFile());
               ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);

               // 2b. define the thread consuming the output sources
               vector<string> valuesToWrite = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques", "AdjustedEmgs" };
               QueuesToStorageFiles queuesToStorageFiles(mySubject, valuesToWrite, outputDirectory);

               // 3. define the model simulator
               vector<string> valuesToLog = { "Activations", "FiberLenghts", "FiberVelocities", "MuscleForces", "Torques", "AdjustedEmgs" };

               CEINMS::InputConnectors::doneWithSubscription.setCount(5);
               CEINMS::OutputConnectors::doneWithExecution.setCount(2);

               // 4. define the optimiser
               typedef Hybrid::ErrorMinimizerAnnealing<MyNMSmodel> MyErrorMinimizer;
               MyErrorMinimizer errorMinimizer(mySubject);
               HybridWeightings weightings;
               executionCfg.getHybridWeightings(weightings.alpha, weightings.beta, weightings.gamma);
               errorMinimizer.setWeightings(weightings);
               vector<string> toPredict, toTrack;
               executionCfg.getMusclesToPredict(toPredict);
               executionCfg.getMusclesToTrack(toTrack);
               errorMinimizer.setMusclesNamesWithEmgToPredict(toPredict);
               errorMinimizer.setMusclesNamesWithEmgToTrack(toTrack);
               double rt, t, epsilon;
               unsigned noEpsilon, ns, nt, maxNoEval;
               executionCfg.getAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);
               errorMinimizer.setAnnealingParameters(nt, ns, rt, t, maxNoEval, epsilon, noEpsilon);

               ModelEvaluationHybrid<MyNMSmodel, MyErrorMinimizer, LoggerOnQueues> simulator(mySubject, errorMinimizer, valuesToLog);


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
               break;
         } 


        default:
            std::cout << "Implementation not available yet. Verify you XML configuration file\n";
            break;
            
                
            
            
    }
    
  
  return 0;
}
