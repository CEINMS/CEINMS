//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef DataSynchronizer_h
#define DataSynchronizer_h

#include <vector>
#include <list>
#include <limits>
#include <algorithm>

#include <iostream>
#include <stdlib.h>

#include "SyncTools.h"

namespace SyncTools {
    
    
    class DataSynchronizer {
    public:
        void getEmgs(std::vector<double>& emgs);
        void getMuscleTendonLengths(std::vector<double>& lmt);
        void getMomentArms(std::vector<double>& ma, unsigned whichDof);
        void getTorques(std::vector<double>& torques);
        void hasTorques(bool hasTorques) { hasTorques_ = hasTorques;}
        bool hasTorques() { return hasTorques_; }
        
    private:
        void popEmgFront(std::vector<double>& emgFromQueue);
        void popLmtFront(std::vector<double>& lmtFromQueue);
        void popMomentArmsFront(std::vector<double>& momentArmsFromQueue, unsigned int whichDof);
        void popExternalTorqueFront(std::vector<double>& externalTorqueFromQueue, unsigned int whichDof);   

        bool hasTorques_;
    };
    
    
void DataSynchronizer::getEmgs(std::vector<double>& emgs) {

    
}

    

    void DataSynchronizer::popEmgFront(std::vector<double>& emgFromQueue) {   
        
        Shared::queueEmgSemFull.wait(); //waits if there is no item in queueEmg
        Shared::queueEmgMutex.lock();   

        emgFromQueue = Shared::queueEmg.front(); 
        Shared::queueEmg.pop_front(); 
        
        Shared::queueEmgMutex.unlock();
        Shared::queueEmgSemEmpty.notify();  //notify that an item has been removed from queueEmg
                                    
    }

    void DataSynchronizer::popLmtFront(std::vector<double>& lmtFromQueue) { 

        Shared::queueLmtSemFull.wait();
        Shared::queueLmtMutex.lock(); 
 
        lmtFromQueue = Shared::queueLmt.front(); 
        Shared::queueLmt.pop_front();  
  
        Shared::queueLmtMutex.unlock();
        Shared::queueLmtSemEmpty.notify();
 
    }

    void DataSynchronizer::popMomentArmsFront(std::vector<double>& momentArmsFromQueue, unsigned int whichDof) { 
  
        Shared::queueMomentArmsSemFull.wait();
        Shared::queueMomentArmsMutex.lock();   

        momentArmsFromQueue = Shared::queueMomentArms.at(whichDof).front(); 
        Shared::queueMomentArms.at(whichDof).pop_front();  

        Shared::queueMomentArmsMutex.unlock();
        Shared::queueMomentArmsSemEmpty.notify();
    }
    
  
    void DataSynchronizer::popExternalTorqueFront(vector<double>& externalTorqueFromQueue, unsigned int whichDof) {   
    
        Shared::queueExternalTorqueSemFull.wait(); //waits if there is no item in queue
        Shared::queueExternalTorqueMutex.lock();   

        externalTorqueFromQueue = (Shared::queueExternalTorque.at(whichDof)).front(); 
        Shared::queueExternalTorque.at(whichDof).pop_front(); 
  
        Shared::queueExternalTorqueMutex.unlock();
        Shared::queueExternalTorqueSemEmpty.notify();  //notify that an item has been removed from queue
    }
};


#endif