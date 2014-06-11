//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "InputQueues.h"
#include "SimpleFileLogger.h"
#include "StorageLogger.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
#include <boost/concept_check.hpp>
using std::string;

#include "ModelEvaluationOnline.h"
#include "InputQueues.h"
#define LOG_FILES
#define LOG

template <typename NMSmodelT>
ModelEvaluationOnline<NMSmodelT>::ModelEvaluationOnline(NMSmodelT& subject, const std::string& outputDir )
:subject_(subject), outputDir_(outputDir)
{
  subject_.getDoFNames(dofNames_);
  noDof_ = dofNames_.size(); 
  globalEmDelay_ = subject.getGlobalEmDelay(); 
}

template <typename NMSmodelT>
void ModelEvaluationOnline<NMSmodelT>::operator()() {

  CEINMS::InputConnectors::queueLmt.subscribe();
  CEINMS::InputConnectors::queueEmg.subscribe();
  for (auto& it : CEINMS::InputConnectors::queueMomentArms)
    (*it).subscribe(); 
  CEINMS::InputConnectors::queueExternalTorques.subscribe();
  
  CEINMS::InputConnectors::doneWithSubscription.wait();
  
#ifdef LOG
  cout << "starting consume" << endl;
#endif
  
#ifdef LOG_FILES
    Logger::StorageLogger<NMSmodelT> logger(subject_, outputDir_);
    //Logger::SimpleFileLogger<NMSmodelT> logger(subject_, outputDir_);
    logger.addLog(Logger::Activations);
    logger.addLog(Logger::FibreLengths);
    logger.addLog(Logger::FibreVelocities);
    logger.addLog(Logger::MuscleForces);
    logger.addLog(Logger::Torques);
#endif
  double externalTorqueTime = std::numeric_limits<double>::lowest();
  bool runCondition = true; 
  do {  // while(runCondition)
     
    // 1. read lmt Data
    CEINMS::InputConnectors::FrameType lmtFrameFromQueue;
    double lmtMaTime;
     
    getLmtFromInputQueue(lmtFrameFromQueue);
    lmtMaTime = lmtFrameFromQueue.time;
#ifdef LOG
    cout << lmtMaTime << endl; 
    for (auto& it: lmtFrameFromQueue.data)
      cout << it << " ";
    cout << endl;
#endif
   
  
    // 2. read moment arms data
    vector< CEINMS::InputConnectors::FrameType > momentArmsFrameFromQueue;
    momentArmsFrameFromQueue.resize(noDof_);
    for(unsigned int i = 0; i < noDof_; ++i) {
      getMomentArmsFromInputQueue((momentArmsFrameFromQueue.at(i)), i); 
    }
    
    // 3. read external Torque 
    CEINMS::InputConnectors::FrameType externalTorquesFrameFromQueue;  

    if (CEINMS::InputConnectors::externalTorquesAvailable) {
        while ((externalTorqueTime < lmtMaTime) /*&& (!(externalTorquesFromQueue.empty()))*/) {
        getExternalTorquesFromInputQueue(externalTorquesFrameFromQueue);
        externalTorqueTime = externalTorquesFrameFromQueue.time;
      }
    }

//:TODO: nota da aggiornare....
//dopo il for la variabile externalTorqueFromQueue conterrà i valori delle torque esterne sincronizzati con i tempi
//forniti dal produttore di lmt e ma. Si ricorda che per questioni di ortogonalità la variabile externalTorqueFromQueue
//è un vettore di vettori di double. tuttavia, poiché è presente un unico valore di torque esterna per un dato istante di tempo
//è possibile estrarre i dati e porli in un semplice vettore di double.
//externalTorqueTime contiene i valori temporali di ogni torque esterna. la forma è generalizzata poiché si ipotizza una diversa frequenza
//di campionamento per ogni torque esterna.
//nel caso attuale externalTorqueTime contine lo stesso valore temporale per ogni torque esterna.
//nella variabile privata dofNamesWithExtTorque_ sono contenuti i nomi dei gradi di libertà ai quali è associata una torque esterna
//tale variabile può essere utilizzata come controllo, poiché la torque esterna è misurata solo su alcuni dof.

    CEINMS::InputConnectors::FrameType emgFrameFromQueue;
    double emgTime;
    do {
      getEmgFromInputQueue(emgFrameFromQueue);
      emgTime = emgFrameFromQueue.time + globalEmDelay_;
      if(!emgFrameFromQueue.data.empty()) {
        //ROBA CHE DEVE FARE EMG
        subject_.setTime(emgTime);
        subject_.setEmgs(emgFrameFromQueue.data);
        if (emgTime < lmtMaTime) {
          subject_.updateActivations();
          subject_.pushState();
        
#ifdef LOG_FILES  
          logger.log(emgTime, Logger::Activations);
#endif  
        }
      } else runCondition = false;
    } while(emgTime < lmtMaTime && runCondition);
   
 //ROBA VARIA lmt ma
    if (!lmtFrameFromQueue.data.empty() && !momentArmsFrameFromQueue.empty() && runCondition) {
      subject_.setMuscleTendonLengths(lmtFrameFromQueue.data);
      for (unsigned int i = 0; i < noDof_; ++i)     
        subject_.setMomentArms(momentArmsFrameFromQueue.at(i).data, i); 
      subject_.updateState();
      subject_.pushState();
    


#ifdef LOG_FILES
      logger.log(emgTime, Logger::Activations);
      logger.log(emgTime, Logger::FibreLengths);
      logger.log(emgTime, Logger::FibreVelocities);
      logger.log(emgTime, Logger::MuscleForces);
      logger.log(emgTime, Logger::Torques);
#endif
      
#ifdef LOG
      cout << endl << endl << "Time: " << emgTime << endl << "EMG" << endl;
      for (auto& it:emgFrameFromQueue.data)
        cout << it << "\t";
       
      cout << endl << "Lmt" << endl;
      for (auto& it:lmtFrameFromQueue.data)
        cout << it << "\t"; 

      for (unsigned int j = 0; j < dofNames_.size(); ++j) {
        cout << endl << "MomentArms on: " << dofNames_.at(j) << endl;
        for (auto& it: momentArmsFrameFromQueue.at(j).data)
          cout << it << "\t"; 
      }
      
      for (auto& it:externalTorquesFrameFromQueue.data)
        cout << it << " "; 
       
      vector<double> cTorques;
      subject_.getTorques(cTorques);
      for(unsigned int i = 0; i < cTorques.size(); ++i) {
        cout << "\nCurrent Torque on " << dofNames_.at(i) << " ";
        cout << cTorques.at(i); 
      }   
      cout << endl << "----------------------------------------" << endl;
#endif
    } else runCondition = false;

//    when time value from input data is greater then globalTimeLimit (which value is set in ExternalVariables.cpp)
//   OR when an empty vector is acqured from one of the queues the thread stop consuming 
// NOTE: when one a producer push an empty vector in a queue means that ther are no more data to be produced, it's like an end frame. 

    runCondition = (emgTime <  CEINMS::InputConnectors::globalTimeLimit) && (lmtMaTime <  CEINMS::InputConnectors::globalTimeLimit) && runCondition;
  } while (runCondition);

#ifdef LOG  
   cout << "Estimation completed. Output file printed in "+outputDir_ << endl;;
#endif
}


