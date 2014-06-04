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
    
    const unsigned int numberOfThreads = 4; // 4 se c'è il produttore della torque esterna, 3 altrimenti
    Concurrency::Queue< std::vector<double> > queueEmg; 
    Concurrency::Queue< std::vector<double> > queueLmt;
    std::vector< Concurrency::Queue< std::vector<double> > > queueMomentArms; 
    std::vector< Concurrency::Queue< std::vector<double> > > queueExternalTorque;
    
    CEINMS::Concurrency::Latch readyInputQueues(numberOfThreads); 
    
    };
};
