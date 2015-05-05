//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_InputConnectors_h
#define ceinms_InputConnectors_h

#include "Queue.h"
#include "Latch.h"
#include "QueueData.h"

#include <list>
#include <vector>
 
namespace ceinms {
    class InputConnectors {
    public:
        InputConnectors();
        virtual ~InputConnectors();
        InputConnectors(const InputConnectors&) = delete;
        typedef QueueData< std::vector< double > > FrameType;
        Concurrency::Queue< FrameType > queueEmg;
        Concurrency::Queue< FrameType > queueLmt;
        std::vector< Concurrency::Queue< FrameType >* > queueMomentArms;
        Concurrency::Queue< FrameType >  queueExternalTorques;
        Concurrency::Latch doneWithSubscription;

        double globalTimeLimit;
        bool externalTorquesAvailable;
    };
}

#endif
