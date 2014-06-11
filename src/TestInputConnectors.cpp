//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "ExternalTorquesFromStorageFile.h"
//#include "ModelEvaluationOnline.h"
//#include "ModelEvaluationOffline.h"
//#include "ModelEvaluationHybrid.h"
#include "SetupDataStructure.h"
#include "Activation/ExponentialActivation.h"
#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"
#include "ErrorMinimizerAnnealing.h"
#include "HybridWeightings.h"
#include "Curve.h"
#include "ExecutionXmlReader.h"

#include "FileUtils.h"

#include <iomanip>

#include <ctime>

#include <string>
using std::string;
#include <iostream>
using std::cout;
#include <vector>
using std::vector;
#include <stdlib.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;



template <typename T>
void setupSubject(T& mySubject, string configurationFile) {
    
    SetupDataStructure<T> setupData(configurationFile);
    setupData.createCurves();
    setupData.createMuscles(mySubject);
    setupData.createDoFs(mySubject);
    
}
        
        


void consumeAndStore(CEINMS::Concurrency::Queue< CEINMS::InputConnectors::FrameType >& queue, const string& outputFileName, const vector<string>& header) {
  
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
     CEINMS::InputConnectors::FrameType item = queue.pop();
     for (auto it : item.data)
       outputFile << std::setprecision(8)  << it << ","; 
     outputFile << item.time <<","; 
     outputFile << endl; 
  }
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

int main(int argc, char** argv) {
 
   
  
    string subjectFile;
    string executionFile;
    string inputDirectory;
    string outputDirectory;
    string emgGeneratorFile;

    int opt;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("subject,s", po::value<string>(&subjectFile), "subject xml file")
    ("execution,x", po::value<string>(&executionFile),  "execution xml file")
    ("input-dir,i", po::value<string>(&inputDirectory), "trial directory path")
    ("output-dir,o", po::value<string>(&outputDirectory)->default_value("./Output"), "output directory")
    ("emg-generator,eg", po::value<string>(&emgGeneratorFile)->default_value("cfg/xml/emgGenerator.xml"), "EMG mapping");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help") || argc < 7) {
        cout << desc << "\n";
        return 1;
    }
      
    typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online> MyNMSmodel;
    MyNMSmodel mySubject;
    setupSubject(mySubject, subjectFile);
    
    vector<string> muscleNames; 
    mySubject.getMuscleNames(muscleNames);
    vector<string> dofNames;
    mySubject.getDoFNames(dofNames);
    
    
    string emgFilename(FileUtils::getFile(inputDirectory, "emg.txt"));
    EMGFromFile emgProducer(mySubject, emgFilename);
    
    
    string externalTorqueFilename(inputDirectory + "inverse_dynamics.sto");
    ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, externalTorqueFilename);  
    
    string lmtFilename;
    vector< string > maFilename;
    setLmtMaFilenames(inputDirectory, dofNames, lmtFilename, maFilename);
    LmtMaFromStorageFile lmtMaProducer(mySubject, lmtFilename, maFilename); 
    
    CEINMS::InputConnectors::doneWithSubscription.setCount(6+dofNames.size());
    
    std::thread emgProdThread(std::ref(emgProducer)); 
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    
    
    std::thread emgConsThread(std::ref(consumeAndStore), std::ref(CEINMS::InputConnectors::queueEmg), outputDirectory + "/emg.csv", std::ref(muscleNames)); 
    std::thread externalTorquesConThread(std::ref(consumeAndStore), std::ref(CEINMS::InputConnectors::queueExternalTorques), outputDirectory + "/externalTorques.csv",std::ref(dofNames));
    std::thread lmtConsThread(std::ref(consumeAndStore), std::ref(CEINMS::InputConnectors::queueLmt),outputDirectory + "/lmt.csv", std::ref(muscleNames));
    
    
   std::vector<std::thread> maConsThreads(dofNames.size());
   vector< vector <string> > muscleNamesOnDofs; 
   mySubject.getMuscleNamesOnDofs(muscleNamesOnDofs);
   for (int currentDof= 0; currentDof < dofNames.size(); ++currentDof) 
   { 
     maConsThreads.at(currentDof) = std::thread(std::ref(consumeAndStore), std::ref(*(CEINMS::InputConnectors::queueMomentArms.at(currentDof))), outputDirectory + "/ma_" + dofNames.at(currentDof) + ".csv", std::ref(muscleNamesOnDofs.at(currentDof)));
   } 
    
    emgProdThread.join();
     lmtMaProdThread.join();
     externalTorquesProdThread.join();
    
    emgConsThread.join();
    externalTorquesConThread.join();
    lmtConsThread.join();
    for (int i = 0; i < dofNames.size(); ++i) {
      maConsThreads.at(i).join(); 
    }
    
  return 0;
}
