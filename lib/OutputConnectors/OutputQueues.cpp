//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "OutputQueues.h"

#include <map>
#include <string>


namespace CEINMS {
  namespace OutputConnectors { 
     
    std::map< std::string, Concurrency::Queue< FrameType >* > logQueues; 
    
    Concurrency::Latch doneWithExecution; 
    };
};

