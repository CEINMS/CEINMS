//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef InputQueues_h
#define InputQueues_h

#include "Queue.h"
#include "Latch.h"
#include "QueueData.h"

#include <list>
#include <vector>
 
 

namespace CEINMS {
  namespace InputConnectors { 
     
    typedef QueueData< std::vector< double > > FrameType; 

    extern float globalTimeLimit;
    
    extern Concurrency::Queue< FrameType > queueEmg; 
    extern Concurrency::Queue< FrameType > queueLmt; 
    extern std::vector< Concurrency::Queue< FrameType >* > queueMomentArms; 
    
    extern bool externalTorquesAvailable; 
    extern Concurrency::Queue< FrameType >  queueExternalTorques;
 
    extern Concurrency::Latch doneWithSubscription; 
    };
};

#endif