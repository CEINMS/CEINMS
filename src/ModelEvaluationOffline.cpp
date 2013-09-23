//#include "ModelEvaluationOffline.h"
#include "SyncTools.h"
#include "SimpleFileLogger.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

#define LOG_FILES
#define LOG



template <typename NMSmodelT>
ModelEvaluationOffline<NMSmodelT>::ModelEvaluationOffline(NMSmodelT& subject) 
:subject_(subject)
{ }


template <typename NMSmodelT>
void ModelEvaluationOffline<NMSmodelT>::initOfflineCurve() {

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
    
    double globalEmDelay = subject_.getGlobalEmDelay(); 
    unsigned lmtCt = 0;
    double lmtTime = (allLmt.at(0)).back();
    double emgTime; 
    
    for(int emgCt = 0; emgCt < allEmg.size() - 1; ++emgCt) {
        emgTime = (allEmg.at(emgCt)).back() + globalEmDelay;
        allEmg.at(emgCt).pop_back();
        subject_.setTime(emgTime);
        subject_.setEmgs(allEmg.at(emgCt));
        subject_.updateActivations();
        if ((lmtTime <= emgTime) && (lmtCt < allLmt.size() - 1)) {
            allLmt.at(lmtCt).pop_back();
            subject_.setMuscleTendonLengths(allLmt.at(lmtCt));
            subject_.updateFibreLengths_OFFLINEPREP();
            ++lmtCt;
            lmtTime = allLmt.at(lmtCt).back();
        }
        subject_.pushState();
    }
    subject_.updateFibreLengthTraces();
}


template <typename NMSmodelT>
void ModelEvaluationOffline<NMSmodelT>::setSubject(NMSmodelT& subject) {
    
    subject_ = subject;
}



template <typename NMSmodelT>
void ModelEvaluationOffline<NMSmodelT>::operator()() {
 
    vector<double> emgFromQueue;
    vector<double> lmtFromQueue;
    vector< vector<double> > momentArmsFromQueue;
    vector< vector<double> > externalTorqueFromQueue;  
    double emgTime, lmtMaTime;
    vector<double> externalTorqueTime;
    bool runCondition = true;
    
    //read dofs names from XML file and set dofNames global variable
    subject_.getDoFNames(dofNames_);
    setDofNamesToShared(dofNames_);  //set dof names on global variable, needed to lmtMaFromFile class
    noDof_ = dofNames_.size();
    momentArmsFromQueue.resize(noDof_);
    
    // all initialization stuff MUST be placed before this line
    // be sure to call getMusclesNames() just after readyToStart.wait() if you want to
    SyncTools::Shared::readyToStart.wait(); //barrier

    vector<string> muscleNamesFromShared;
    getMusclesNamesFromShared(muscleNamesFromShared);
    vector< vector<string> > muscleNamesOnDofFromShared;
    getMusclesNamesOnDofsFromShared(muscleNamesOnDofFromShared);
 
//CHECK MUSCLES NAMES between XML model and input files 

    getDofNamesAssociatedToExternalTorque(dofNamesWithExtTorque_);
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
  
    if(!subject_.compareMusclesNames(muscleNamesFromShared)) {
        cout << "ERROR: muscles names from emg or lmt files don't match with XML model ones\n";
        exit(EXIT_FAILURE);
    }
    
    for(unsigned int i = 0; i < noDof_; ++i) {
        if(!subject_.compareMusclesNamesOnDoF(muscleNamesOnDofFromShared.at(i), i)) {
        cout << "ERROR: muscles names from " << dofNames_.at(i) << "Ma.txt file don't match with XML model ones\n";
        exit(EXIT_FAILURE);
        }      
    }

//END CHECK MUSCLES

    double globalEmDelay = subject_.getGlobalEmDelay(); 

#ifdef LOG
  cout << "starting consume" << endl;
#endif
  
#ifdef LOG_FILES
    Logger::SimpleFileLogger<NMSmodelT> logger(subject_);
    logger.addLog(Logger::Activations);
    logger.addLog(Logger::FibreLengths);
    logger.addLog(Logger::FibreVelocities);
    logger.addLog(Logger::MuscleForces);
    logger.addLog(Logger::Torques);
#endif

    initOfflineCurve();
    
    vector<bool> stillExtTorqueDataOnDof;
    for (unsigned int i = 0; i < dofNamesWithExtTorque_.size(); ++i)
        stillExtTorqueDataOnDof.push_back(true);

    do {  
        getLmtFromShared(lmtFromQueue);
        lmtMaTime = lmtFromQueue.back();
        lmtFromQueue.pop_back();         //removes time value from the end of vector
        for(unsigned int i = 0; i < noDof_; ++i) {
            getMomentArmsFromShared((momentArmsFromQueue.at(i)), i);    
            momentArmsFromQueue.at(i).pop_back();  //removes time value from the end of vector
        }
        for(unsigned int i = 0; i < dofNamesWithExtTorque_.size(); ++i) {  
            do {
//poiché i dati delle torque esterne provenienti dal produttore possono avere frequenze di campionamento diverse
//è allo stesso modo possibile avere un numero di dati differente a seconda del dof considerato
//stillExtTorqueDataOnDof è un vettore che tiene conto di questo, in modo da evitare problemi di lunghezza sui vettori
                if(stillExtTorqueDataOnDof.at(i)) {   
                    getExternalTorqueFromShared(externalTorqueFromQueue.at(i), i);
                    externalTorqueTime.at(i) = externalTorqueFromQueue.at(i).back();
                    externalTorqueFromQueue.at(i).pop_back();
                    if(externalTorqueFromQueue.at(i).empty())
                        stillExtTorqueDataOnDof.at(i) = false;
                }
      
            } while( (externalTorqueTime.at(i) < lmtMaTime) && stillExtTorqueDataOnDof.at(i));     
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

        do {
            getEmgFromShared(emgFromQueue);
            emgTime = emgFromQueue.back() + globalEmDelay;
            emgFromQueue.pop_back();
            if(!emgFromQueue.empty()) {
                //ROBA CHE DEVE FARE EMG
                subject_.setTime(emgTime);
                subject_.setEmgs(emgFromQueue);
                if(emgTime < lmtMaTime) {
                    subject_.updateActivations();
                    subject_.pushState();
        
#ifdef LOG_FILES  
                    logger.log(emgTime, Logger::Activations);
#endif  
                }
            }
            else runCondition = false;
        } while( (emgTime < lmtMaTime) && runCondition);
   
 //ROBA VARIA lmt ma
        if (!lmtFromQueue.empty() && !momentArmsFromQueue.empty() && runCondition) {
            subject_.setMuscleTendonLengths(lmtFromQueue);
            for(unsigned int i = 0; i < noDof_; ++i)     
                subject_.setMomentArms(momentArmsFromQueue.at(i), i); 
            subject_.updateState_OFFLINE();
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
        for(unsigned int i=0; i < emgFromQueue.size(); ++i)
            cout << emgFromQueue.at(i) << "\t" ;
        cout << endl << "Lmt" << endl;
        for(unsigned int i=0; i < lmtFromQueue.size(); ++i)
            cout << lmtFromQueue.at(i) << "\t";

        for(unsigned int j = 0; j < dofNames_.size(); ++j) {
            cout << endl << "MomentArms on " << dofNames_.at(j) << endl;
            for(unsigned int i=0; i < (momentArmsFromQueue.at(j)).size(); ++i)
                cout << (momentArmsFromQueue.at(j)).at(i) << "\t";
        }
      
        for(unsigned int i = 0; i < dofNamesWithExtTorque_.size(); ++i) {
            if(!externalTorqueFromQueue.at(i).empty()) {
                cout << "\nExternal Torque on " << dofNamesWithExtTorque_.at(i) << " ";
                cout << externalTorqueFromQueue.at(i).at(0); 
            }        
        }   
        vector<double> cTorques;
        subject_.getTorques(cTorques);
        for(unsigned int i = 0; i < cTorques.size(); ++i) {
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
    } while (runCondition);

#ifdef LOG  
  cout << "Everything went fine, check output files in ./Output\n";
#endif
}


template <typename NMSmodelT>
ModelEvaluationOffline<NMSmodelT>::~ModelEvaluationOffline()
{

}









