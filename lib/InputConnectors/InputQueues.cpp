//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "InputQueues.h"

#include <vector>


namespace CEINMS {
  namespace InputConnectors {
    //:TODO: I have no idea about this globalTimeLimit.... WHY... AND WHY SO HIDDEN.. come on
    float globalTimeLimit = 100.;
    
    Concurrency::Queue< FrameType > queueEmg; 
    Concurrency::Queue< FrameType > queueLmt; 
    std::vector< Concurrency::Queue< FrameType >* > queueMomentArms; 
    
    bool externalTorquesAvailable = false; 
    Concurrency::Queue< FrameType >  queueExternalTorques;
 
    Concurrency::Latch doneWithSubscription; 
  
    
    
    
    };
};
