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
           
    EMGFromFile emgProducer(mySubject, inputDirectory);
    LmtMaFromStorageFile lmtMaProducer(mySubject, inputDirectory);
    ExternalTorquesFromStorageFile externalTorquesProducer(mySubject, inputDirectory);   
    
    vector<string> muscleNames; 
    mySubject.getMuscleNames(muscleNames);
    vector<string> dofNames;
    mySubject.getDoFNames(dofNames);
    CEINMS::InputConnectors::inputQueuesAreReady.setCount(4); 
    CEINMS::InputConnectors::doneWithSubscription.setCount(6+dofNames.size());
    
    std::thread emgProdThread(emgProducer); 
    std::thread externalTorquesProdThread(externalTorquesProducer);
    std::thread lmtMaProdThread(lmtMaProducer);
    CEINMS::InputConnectors::inputQueuesAreReady.wait();
    
    
     std::thread emgConsThread(consumeAndStore, ref(CEINMS::InputConnectors::queueEmg), outputDirectory + "/emg.csv", ref(muscleNames)); 
     std::thread externalTorquesConThread(consumeAndStore, ref(CEINMS::InputConnectors::queueExternalTorques), outputDirectory + "/externalTorques.csv",ref(dofNames));
     std::thread lmtConsThread(consumeAndStore, ref(CEINMS::InputConnectors::queueLmt),outputDirectory + "/lmt.csv", ref(muscleNames));
    
    
    std::vector<std::thread> maConsThreads(dofNames.size());
    vector< vector <string> > muscleNamesOnDofs; 
    mySubject.getMuscleNamesOnDofs(muscleNamesOnDofs);
    for (int currentDof= 0; currentDof < dofNames.size(); ++currentDof) 
    { 
      maConsThreads.at(currentDof) = std::thread(consumeAndStore, ref(*(CEINMS::InputConnectors::queueMomentArms.at(currentDof))), outputDirectory + "/ma_" + dofNames.at(currentDof) + ".csv", ref(muscleNamesOnDofs.at(currentDof)));
    } 
    
    emgProdThread.join();
    externalTorquesProdThread.join();
    lmtMaProdThread.join();
    
      emgConsThread.join();
    externalTorquesConThread.join();
    lmtConsThread.join();
    for (int i = 0; i < dofNames.size(); ++i) {
      maConsThreads.at(i).join(); 
    }
    
  return 0;
}
