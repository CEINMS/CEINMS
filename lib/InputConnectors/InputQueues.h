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

#include <list>
#include <vector>
 
namespace CEINMS {
  namespace InputConnectors { 
    
    extern float globalTimeLimit;
    
    extern Concurrency::Queue< std::vector<double> > queueEmg; 
    extern Concurrency::Queue< std::vector<double> > queueLmt; 
    extern std::vector< Concurrency::Queue< std::vector<double> > > queueMomentArms; 
    extern std::vector< Concurrency::Queue< std::vector<double> > > queueExternalTorque;
 
    extern Concurrency::Latch readyInputQueues; 
    };
};

#endif