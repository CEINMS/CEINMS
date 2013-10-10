//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "ModelEvaluationBase.h"
#include "SyncTools.h"

#include <vector>
using std::vector;
#include <string>
using std::string;

void ModelEvaluationBase::getEmgFromShared(vector<double>& emgs) {

    SyncTools::Shared::queueEmgSemFull.wait(); //waits if there is no item in queueEmg
    SyncTools::Shared::queueEmgMutex.lock();   

    emgs = SyncTools::Shared::queueEmg.front(); 
    SyncTools::Shared::queueEmg.pop_front(); 
    
    SyncTools::Shared::queueEmgMutex.unlock();
    SyncTools::Shared::queueEmgSemEmpty.notify();  //notify that an item has been removed from queueEmg
}

void ModelEvaluationBase::getLmtFromShared(vector<double>& lmts) {

    SyncTools::Shared::queueLmtSemFull.wait();
    SyncTools::Shared::queueLmtMutex.lock(); 
    
    lmts = SyncTools::Shared::queueLmt.front(); 
    SyncTools::Shared::queueLmt.pop_front();  
    
    SyncTools::Shared::queueLmtMutex.unlock();
    SyncTools::Shared::queueLmtSemEmpty.notify();
}


void ModelEvaluationBase::getMomentArmsFromShared(vector<double>& momentArms, unsigned int whichDof) {
    
    SyncTools::Shared::queueMomentArmsSemFull.wait();
    SyncTools::Shared::queueMomentArmsMutex.lock();   

    momentArms = SyncTools::Shared::queueMomentArms.at(whichDof).front(); 
    SyncTools::Shared::queueMomentArms.at(whichDof).pop_front();  

    SyncTools::Shared::queueMomentArmsMutex.unlock();
    SyncTools::Shared::queueMomentArmsSemEmpty.notify();
}


void ModelEvaluationBase::getExternalTorqueFromShared(vector<double>& externalTorque, unsigned int whichDof) {

    SyncTools::Shared::queueExternalTorqueSemFull.wait(); //waits if there is no item in queue
    SyncTools::Shared::queueExternalTorqueMutex.lock();   

    externalTorque = SyncTools::Shared::queueExternalTorque.at(whichDof).front(); 
    SyncTools::Shared::queueExternalTorque.at(whichDof).pop_front(); 
    
    SyncTools::Shared::queueExternalTorqueMutex.unlock();
    SyncTools::Shared::queueExternalTorqueSemEmpty.notify();  //notify that an item has been removed from queue
}


void ModelEvaluationBase::getDofNamesAssociatedToExternalTorque(vector<string>& dofNames) {

    SyncTools::Shared::dofNamesWithExtTorqueMutex.lock();
    dofNames = SyncTools::Shared::dofNamesWithExtTorque; //make a local copy of global variable dofNamesWithExtTorque
    SyncTools::Shared::dofNamesWithExtTorqueMutex.unlock();
}


void ModelEvaluationBase::getMusclesNamesFromShared(vector<string>& muscleNames) {
    
    SyncTools::Shared::musclesNamesMutex.lock();
    muscleNames = SyncTools::Shared::musclesNames; //make a local copy of global variable musclesNames
    SyncTools::Shared::musclesNamesMutex.unlock();
}


void ModelEvaluationBase::getMusclesNamesOnDofsFromShared(vector< vector<string> >& muscleNamesOnDofs){
    
    SyncTools::Shared::musclesNamesOnDofMutex.lock();
    muscleNamesOnDofs = SyncTools::Shared::musclesNamesOnDof; //make a local copy of global variable musclesNamesOnDof
    SyncTools::Shared::musclesNamesOnDofMutex.unlock();
}


//controllare se viene usata
void ModelEvaluationBase::setDofNamesToShared(const vector<string>& dofNames) {

    SyncTools::Shared::dofNames = dofNames; //dofNames is a global variable, it's needed for LmtMaFromFile class
    SyncTools::Shared::dofNamesSem.notify();
}



ModelEvaluationBase::~ModelEvaluationBase() { }




