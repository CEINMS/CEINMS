//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef InputConnectors_h
#define InputConnectors_h

#include "Queue.h"
#include "Latch.h"
#include "QueueData.h"

#include <list>
#include <vector>
 
 

namespace CEINMS {
  class InputConnectors {
  public:
      InputConnectors(){};
    ~InputConnectors();
    InputConnectors(const InputConnectors& inputConn) = delete;
    typedef QueueData< std::vector< double > > FrameType;

    float globalTimeLimit;

    Concurrency::Queue< FrameType > queueEmg;
    Concurrency::Queue< FrameType > queueLmt;
    std::vector< Concurrency::Queue< FrameType >* > queueMomentArms;
    
    bool externalTorquesAvailable;
    Concurrency::Queue< FrameType >  queueExternalTorques;
 
    Concurrency::Latch doneWithSubscription;
    };
};

#endif
