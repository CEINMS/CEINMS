//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef SyncTools_h
#define SyncTools_h

#include "Queue.h"
#include "Latch.h"

#include <string>
#include <list>
#include <vector>

using namespace CEINMS; 
namespace SyncTools {
  namespace Shared{
    extern float globalTimeLimit;
    extern const int queueBuffer;
    
//    extern Semaphore lmtProducingDone;
//    extern Semaphore emgProducingDone;
    };
};

#endif
