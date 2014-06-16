//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "ModelEvaluation.h"
#include "SyncTools.h"
#include "Semaphore.h"
#include "NMSmodel.h" 

#include "SetupDataStructure.h"
//#include "ErrorMinimizer.h"
#include "ExponentialActivation.h"
#include "StiffTendon.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <stdlib.h>
#include <fstream>
using std::ofstream;

#include "SimpleFileLogger.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>

#define LOG
#define LOG_FILES
#define DEBUG_DATA_STRUCTURE


ModelEvaluation::ModelEvaluation(const string& configurationFile)
                :configurationFile_(configurationFile) { }

void ModelEvaluation::popEmgFront(vector<double>& emgFromQueue)
{   
  SyncTools::Shared::queueEmgSemFull.wait(); //waits if there is no item in queueEmg
  SyncTools::Shared::queueEmgMutex.lock();   

  emgFromQueue = SyncTools::Shared::queueEmg.front(); 
  SyncTools::Shared::queueEmg.pop_front(); 
  
  SyncTools::Shared::queueEmgMutex.unlock();
  SyncTools::Shared::queueEmgSemEmpty.notify();  //notify that an item has been removed from queueEmg
                              
}

void ModelEvaluation::popLmtFront(vector<double>& lmtFromQueue)
{ 

  SyncTools::Shared::queueLmtSemFull.wait();
  SyncTools::Shared::queueLmtMutex.lock(); 
 
  lmtFromQueue = SyncTools::Shared::queueLmt.front(); 
  SyncTools::Shared::queueLmt.pop_front();  
  
  SyncTools::Shared::queueLmtMutex.unlock();
  SyncTools::Shared::queueLmtSemEmpty.notify();
 
}

void ModelEvaluation::popMomentArmsFront(vector<double>& momentArmsFromQueue, unsigned int whichDof)
{ 
  SyncTools::Shared::queueMomentArmsSemFull.wait();
  SyncTools::Shared::queueMomentArmsMutex.lock();   

  momentArmsFromQueue = SyncTools::Shared::queueMomentArms.at(whichDof).front(); 
  SyncTools::Shared::queueMomentArms.at(whichDof).pop_front();  

  SyncTools::Shared::queueMomentArmsMutex.unlock();
  SyncTools::Shared::queueMomentArmsSemEmpty.notify();
}

void ModelEvaluation::popExternalTorqueFront(vector<double>& externalTorqueFromQueue, unsigned int whichDof)
{   
  SyncTools::Shared::queueExternalTorqueSemFull.wait(); //waits if there is no item in queue
  SyncTools::Shared::queueExternalTorqueMutex.lock();   

  externalTorqueFromQueue = (SyncTools::Shared::queueExternalTorque.at(whichDof)).front(); 
  SyncTools::Shared::queueExternalTorque.at(whichDof).pop_front(); 
  
  SyncTools::Shared::queueExternalTorqueMutex.unlock();
  SyncTools::Shared::queueExternalTorqueSemEmpty.notify();  //notify that an item has been removed from queue
                              
}


void ModelEvaluation::getMusclesNames()
{
  SyncTools::Shared::musclesNamesMutex.lock();
  musclesNames_ = SyncTools::Shared::musclesNames; //make a local copy of global variable musclesNames
  SyncTools::Shared::musclesNamesMutex.unlock();
  SyncTools::Shared::musclesNamesOnDofMutex.lock();
  musclesNamesOnDof_ = SyncTools::Shared::musclesNamesOnDof; //make a local copy of global variable musclesNamesOnDof
  SyncTools::Shared::musclesNamesOnDofMutex.unlock();
}

void ModelEvaluation::getDofNamesAssociatedToExternalTorque()
{
  SyncTools::Shared::dofNamesWithExtTorqueMutex.lock();
  dofNamesWithExtTorque_ = SyncTools::Shared::dofNamesWithExtTorque; //make a local copy of global variable dofNamesWithExtTorque
  SyncTools::Shared::dofNamesWithExtTorqueMutex.unlock();
}



void ModelEvaluation::setDofNames()
{
  SyncTools::Shared::dofNames = dofNames_; //dofNames is a global variable, it's needed for LmtMaFromFile class
  SyncTools::Shared::dofNamesSem.notify();
}  


void ModelEvaluation::initOfflineCurve(MyNMSmodel& mySubject) {
      
  SyncTools::Shared::lmtProducingDone.wait();
  cout << "fiber velocity CURVE \n";
  vector< vector<double> > allLmt;
  
  SyncTools::Shared::queueLmtMutex.lock();
  std::list< vector<double> >::iterator lmtIt;
  for(lmtIt = SyncTools::Shared::queueLmt.begin(); lmtIt != SyncTools::Shared::queueLmt.end(); lmtIt++)
    allLmt.push_back(*lmtIt);
  SyncTools::Shared::queueLmtMutex.unlock();

  SyncTools::Shared::emgProducingDone.wait();
  vector< vector<double> > allEmg;
  
  SyncTools::Shared::queueEmgMutex.lock();
  std::list< vector<double> >::iterator emgIt;
  for(emgIt = SyncTools::Shared::queueEmg.begin(); emgIt != SyncTools::Shared::queueEmg.end(); emgIt++)
    allEmg.push_back(*emgIt);

  SyncTools::Shared::queueEmgMutex.unlock();
  
  unsigned lmtCt = 0;
  double lmtTime = (allLmt.at(0)).back();
  double emgTime; 
  
  for(int emgCt = 0; emgCt < allEmg.size() - 1; ++emgCt)
  {
    emgTime = (allEmg.at(emgCt)).back();
    allEmg.at(emgCt).pop_back();
    mySubject.setTime(emgTime);
    mySubject.setEmgs(allEmg.at(emgCt));
    mySubject.updateActivations();
    if ((lmtTime <= emgTime) && (lmtCt < allLmt.size() - 1)) {
      allLmt.at(lmtCt).pop_back();
      mySubject.setMuscleTendonLengths(allLmt.at(lmtCt));
      mySubject.updateFibreLengths_OFFLINEPREP();
      ++lmtCt;
      lmtTime = allLmt.at(lmtCt).back();
    }
    mySubject.pushState();
   }
  mySubject.updateFibreLengthTraces();

}


void ModelEvaluation::operator()()
{

  vector<double> emgFromQueue;
  vector<double> lmtFromQueue;
  vector< vector<double> > momentArmsFromQueue;
  vector< vector<double> > externalTorqueFromQueue;  
  double emgTime, lmtMaTime;
  vector<double> externalTorqueTime;
  bool runCondition = true;



    // STEP 1. CREATE THE NMS MODEL FOR THE SUBJECT
    //         BASED ON THE "CALIBRATED" CONFIGURATION FROM configurationFile_
  
    // just check that configurationFile_ is a proper xml
  try {
    std::auto_ptr<NMSmodelType> subjectPointer (subject (configurationFile_));
  }  
  catch (const xml_schema::exception& e) {
    cout << e << endl;
    exit(EXIT_FAILURE);
  }
   
    // create the NMS model of the subject based on a configurationFile_ XML file 
#ifdef LOG 
  cout << "Create data structure...\n";
#endif
  MyNMSmodel mySubject;  
  
//  ErrorMinimizer torqueErrorMin(mySubject);
  
  MySetupDataStructure setupData(configurationFile_); 
    // setup Curves
  cout << "Create curves...\n";
  setupData.createCurves();
    // setup Muscles
  cout << "Create muscles...\n";
  setupData.createMuscles(mySubject);
    // setup DoF
  cout << "Create DoF...\n";
  setupData.createDoFs(mySubject);

  
  //read dofs names from XML file and set dofNames global variable
  mySubject.getDoFNames(dofNames_);
  setDofNames();  //set dof names on global variable, needed to lmtMaFromFile class
  noDof_ = dofNames_.size();
  momentArmsFromQueue.resize(noDof_);

 
  // all initialization stuff MUST be placed before this line
  // be sure to call getMusclesNames() just after readyToStart.wait() if you want to
  SyncTools::Shared::readyToStart.wait(); //barrier

  getMusclesNames();
 
//CHECK MUSCLES NAMES between XML model and input files 

  vector<string> dofNames; //prob non serve a niente questa riga
  mySubject.getDoFNames(dofNames); //e nemmeno questa
  getDofNamesAssociatedToExternalTorque();
  externalTorqueTime.resize(dofNamesWithExtTorque_.size());
  externalTorqueFromQueue.resize(dofNamesWithExtTorque_.size());

#ifdef LOG
  cout << "Deegres of Freedom with an external torque data:\n";
  for(unsigned int i = 0; i < dofNamesWithExtTorque_.size(); ++i)
    cout << dofNamesWithExtTorque_.at(i) << endl;
  if (dofNamesWithExtTorque_.size() == 0)
    cout << "no external torque data found!\n";
#endif

  //external torques provided as input may refer to a subset of dof, 
  //ie. we may have less external torques then the number of dofs 
  //private class variable dofNamesWithExtTorque_ stores the dof names list with external torque data
  
  if(!mySubject.compareMusclesNames(musclesNames_))
  {
    cout << "ERROR: muscles names from emg or lmt files don't match with XML model ones\n";
    exit(EXIT_FAILURE);
  }
  for(unsigned int i = 0; i < noDof_; ++i)
  {
    if(!mySubject.compareMusclesNamesOnDoF(musclesNamesOnDof_.at(i), i))
    {
      cout << "ERROR: muscles names from " << dofNames_.at(i) << "Ma.txt file don't match with XML model ones\n";
      exit(EXIT_FAILURE);
    }      
  }

//END CHECK MUSCLES

#ifdef LOG
  cout << "starting consume" << endl;
#endif
  
#ifdef LOG_FILES
    Logger::SimpleFileLogger<MyNMSmodel> logger(mySubject);
    logger.addLog(Logger::Activations);
    logger.addLog(Logger::FibreLengths);
    logger.addLog(Logger::FibreVelocities);
    logger.addLog(Logger::MuscleForces);
    logger.addLog(Logger::Torques);
#endif

// COMPUTE_FIBER_VELOCITY_CURVE_OFFLINE

  initOfflineCurve(mySubject);

// END COMPUTE_FIBER_VELOCITY_CURVE_OFFLINE  


  vector<bool> stillExtTorqueDataOnDof;
  for (unsigned int i = 0; i < dofNamesWithExtTorque_.size(); ++i)
    stillExtTorqueDataOnDof.push_back(true);


  do
  {  
    popLmtFront(lmtFromQueue);
    lmtMaTime = lmtFromQueue.back();
    lmtFromQueue.pop_back();         //removes time value from the end of vector
    for(unsigned int i = 0; i < noDof_; ++i) 
    {    
      popMomentArmsFront((momentArmsFromQueue.at(i)), i);    
      (momentArmsFromQueue.at(i)).pop_back();  //removes time value from the end of vector
    }



    for(unsigned int i = 0; i < dofNamesWithExtTorque_.size(); ++i)
    {  
      do
      {
//poiché i dati delle torque esterne provenienti dal produttore possono avere frequenze di campionamento diverse
//è allo stesso modo possibile avere un numero di dati differente a seconda del dof considerato
//stillExtTorqueDataOnDof è un vettore che tiene conto di questo, in modo da evitare problemi di lunghezza sui vettori
        if(stillExtTorqueDataOnDof.at(i))
        {   
         popExternalTorqueFront(externalTorqueFromQueue.at(i), i);
         externalTorqueTime.at(i) = externalTorqueFromQueue.at(i).back();
         externalTorqueFromQueue.at(i).pop_back();
         if(externalTorqueFromQueue.at(i).empty())
           stillExtTorqueDataOnDof.at(i) = false;
        }
      
      }
      while( (externalTorqueTime.at(i) < lmtMaTime) && stillExtTorqueDataOnDof.at(i));     
  }


//dopo il for la variabile externalTorqueFromQueue conterrà i valori delle torque esterne sincronizzati con i tempi
//forniti dal produttore di lmt e ma. Si ricorda che per questioni di ortogonalità la variabile externalTorqueFromQueue
//è un vettore di vettori di double. tuttavia, poiché è presente un unico valore di torque esterna per un dato istante di tempo
//è possibile estrarre i dati e porli in un semplice vettore di double.
//externalTorqueTime contiene i valori temporali di ogni torque esterna. la forma è generalizzata poiché si ipotizza una diversa frequenza
//di campionamento per ogni torque esterna.
//nel caso attuale externalTorqueTime contine lo stesso valore temporale per ogni torque esterna.
//nella variabile privata dofNamesWithExtTorque_ sono contenuti i nomi dei gradi di libertà ai quali è associata una torque esterna
//tale variabile può essere utilizzata come controllo, poiché la torque esterna è misurata solo su alcuni dof.

    do
    {
      popEmgFront(emgFromQueue);
      emgTime = emgFromQueue.back();
      emgFromQueue.pop_back();
      if(!emgFromQueue.empty())
      {
      //ROBA CHE DEVE FARE EMG
        mySubject.setTime(emgTime);
        mySubject.setEmgs(emgFromQueue);
        if(emgTime < lmtMaTime) {
            mySubject.updateActivations();
            mySubject.pushState();
        
#ifdef LOG_FILES  
            logger.log(emgTime, Logger::Activations);
#endif  
        }
      }
      else runCondition = false;
    }
    while(emgTime < lmtMaTime);
   
 //ROBA VARIA lmt ma
    if (!lmtFromQueue.empty() && !momentArmsFromQueue.empty())
    {
      mySubject.setMuscleTendonLengths(lmtFromQueue);

      for(unsigned int i = 0; i < noDof_; ++i)     
        mySubject.setMomentArms(momentArmsFromQueue.at(i), i); 

      mySubject.updateState_OFFLINE();
      mySubject.pushState();
    


#ifdef LOG_FILES
        logger.log(emgTime, Logger::Activations);
        logger.log(emgTime, Logger::FibreLengths);
        logger.log(emgTime, Logger::FibreVelocities);
        logger.log(emgTime, Logger::MuscleForces);
        logger.log(emgTime, Logger::Torques);
#endif    


#ifdef LOG
        cout << endl << endl << "Time: " << emgTime << endl << "EMG" << endl;
        for(unsigned int i=0; i < emgFromQueue.size(); ++i)
          cout << emgFromQueue.at(i) << "\t" ;
        cout << endl << "Lmt" << endl;
        for(unsigned int i=0; i < lmtFromQueue.size(); ++i)
          cout << lmtFromQueue.at(i) << "\t";

        for(unsigned int j = 0; j < dofNames_.size(); ++j)
        {
        cout << endl << "MomentArms on " << dofNames_.at(j) << endl;
          for(unsigned int i=0; i < (momentArmsFromQueue.at(j)).size(); ++i)
            cout << (momentArmsFromQueue.at(j)).at(i) << "\t";
        }
      
        for(unsigned int i = 0; i < dofNamesWithExtTorque_.size(); ++i)
        {
          if(!externalTorqueFromQueue.at(i).empty())
          {
            cout << "\nExternal Torque on " << dofNamesWithExtTorque_.at(i) << " ";
            cout << externalTorqueFromQueue.at(i).at(0); 
          }        
        }   
        vector<double> cTorques;
        mySubject.getTorques(cTorques);
        for(unsigned int i = 0; i < cTorques.size(); ++i)
        {
            cout << "\nCurrent Torque on " << dofNames_.at(i) << " ";
            cout << cTorques.at(i); 
                 
        }   
        
        cout << endl << "----------------------------------------" << endl;
#endif
    }
    else runCondition = false;

/*    when time value from input data is greater then globalTimeLimit (which value is set in ExternalVariables.cpp)
   OR when an empty vector is acqured from one of the queues the thread stop consuming 
NOTE: when one a producer push an empty vector in a queue means that ther are no more data to be produced, it's like an end frame. 
*/
    runCondition = (emgTime < SyncTools::Shared::globalTimeLimit) && (lmtMaTime < SyncTools::Shared::globalTimeLimit) && runCondition;
  }
  while (runCondition);

#ifdef LOG  
  cout << "Everything went fine, check output files in ./Output\n";
#endif
}

ModelEvaluation::~ModelEvaluation() {}

