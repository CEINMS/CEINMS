#include "EMGFromFile.h"
#include "LmtMaFromFile.h"
#include "ExternalTorqueFromFile.h"
#include "ModelEvaluationOnline.h"
#include "ModelEvaluationOffline.h"
#include "ModelEvaluationHybrid.h"
#include "SetupDataStructure.h"
#include "Activation/ExponentialActivation.h"
#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"
#include "ErrorMinimizerAnnealing.h"
#include "HybridWeightings.h"
#include "Curve.h"
#include "ExecutionXmlReader.h"

#include <ctime>

#include <boost/thread.hpp>
#include <string>
using std::string;
#include <iostream>
using std::cout;
#include <vector>
using std::vector;


template<typename T1, typename T2, typename T3, typename T4>
void runThreads(T1& t1, T2& t2, T3& t3, T4& t4) {
 
    boost::thread thread1(t1);
    boost::thread thread2(t2);  
    boost::thread thread3(t3);  
    boost::thread thread4(t4);
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
}

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
    cout << "David Lloyd, Monica Reggiani, Massimo Sartori, Claudio Pizzolato\n\n";
}
    

int main(int argc, char** argv) {
 
    printHeader();
    printAuthors();
    
#ifdef LOG  
  cout << "Check configuration data...\n";
#endif
  
    // check command line arguments... 
  if ( argc != 4 ) {
    cout << "Usage: CEINMS configurationFile DirFiles\n";
    cout << "Ex: CEINMS subject.xml execution.xml inputDirectory/ \n";
    exit(EXIT_FAILURE);
  }
  


    EMGFromFile emgProducer(argv[3]);
    LmtMaFromFile lmtMaProducer(argv[3]);
    ExternalTorqueFromFile externalTorqueProducer(argv[3]);
 
    string configurationFile(argv[1]);
    try {
        std::auto_ptr<SubjectType> subjectPointer (subject (configurationFile));
    }  
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
    
    ExecutionXmlReader executionCfg(argv[2]);             
    
   NMSModelCfg::RunMode runMode = executionCfg.getRunMode();
   
//    NMSModelCfg::RunMode runMode = NMSModelCfg::HybridPiecewiseActivationElasticTendonOnline;
    switch(runMode) {
               
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOnline: {
            typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, configurationFile);
            ModelEvaluationOnline<MyNMSmodel> consumer(mySubject);
            runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
            break;
        }
       
        case NMSModelCfg::OpenLoopExponentialActivationStiffTendonOffline: {
            typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, configurationFile);
            ModelEvaluationOffline<MyNMSmodel> consumer(mySubject);
            runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
            break;
        }
        /*
        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonOnline: {
            typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Online>, CurveMode::Online> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, configurationFile);
            ModelEvaluationOnline<MyNMSmodel> consumer(mySubject);
            runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
            break;
        }
        
        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonOffline: {
            typedef NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, configurationFile);
            ModelEvaluationOffline<MyNMSmodel> consumer(mySubject);
            runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
            break;
        }
        */
        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOnline: {
            typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, configurationFile);
            ModelEvaluationOnline<MyNMSmodel> consumer(mySubject);
            runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
            break;
        }
        
        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline: {
            typedef NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline> MyNMSmodel;
            MyNMSmodel mySubject;
            setupSubject(mySubject, configurationFile);
            ModelEvaluationOffline<MyNMSmodel> consumer(mySubject);
            runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
            break;
        }
        
        
        case NMSModelCfg::HybridExponentialActivationStiffTendonOnline: { 
            typedef NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online> MyNMSmodel;
            typedef Hybrid::ErrorMinimizerAnnealing<MyNMSmodel> MyErrorMinimizer;
            SetupDataStructure<MyNMSmodel> setupData(configurationFile);
            MyNMSmodel mySubject;
            setupData.createCurves();
            setupData.createMuscles(mySubject);
            setupData.createDoFs(mySubject);
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
            ModelEvaluationHybrid<MyNMSmodel, MyErrorMinimizer> consumer(mySubject, errorMinimizer);
            runThreads(consumer, emgProducer, lmtMaProducer, externalTorqueProducer);
            break;
        } 


        default:
            std::cout << "Implementation not available yet. Verify you XML configuration file\n";
            break;
            
                
            
            
    }
    
  
  return 0;
}
