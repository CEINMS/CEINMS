//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef OutputQueues_h
#define OutputQueues_h

#include "Queue.h"
#include "Latch.h"
#include "QueueData.h"

#include <map>
#include <string>
 
namespace CEINMS {
  namespace OutputConnectors { 
    
    typedef std::vector< double > DataType; 
    typedef QueueData< DataType > FrameType; 
    
    const double TimePlaceholderForEndOfData = std::numeric_limits<double>::infinity();
    
    extern std::map< std::string, Concurrency::Queue< FrameType >* > logQueues; 
 
    extern Concurrency::Latch doneWithExecution; 
    };
};

#endif