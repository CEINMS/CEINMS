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

#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition_variable.hpp>

#include "Semaphore.h"

#include <string>
#include <list>
#include <vector>

namespace SyncTools {
    namespace Shared{
    extern float globalTimeLimit;
    extern const int queueBuffer;
    extern const unsigned int numberOfThreads;

    extern std::list< std::vector <double> > queueEmg;
    extern boost::mutex queueEmgMutex;
    extern Semaphore queueEmgSemFull, queueEmgSemEmpty;

    extern std::list< std::vector <double> > queueLmt;
    extern boost::mutex queueLmtMutex;
    extern Semaphore queueLmtSemFull, queueLmtSemEmpty;

    extern std::vector< std::list< std::vector <double> > > queueMomentArms;
    extern boost::mutex queueMomentArmsMutex;
    extern Semaphore queueMomentArmsSemFull, queueMomentArmsSemEmpty;

    extern std::vector< std::list< std::vector <double> > > queueExternalTorque;
    extern boost::mutex queueExternalTorqueMutex;
    extern Semaphore queueExternalTorqueSemFull, queueExternalTorqueSemEmpty;

    extern std::vector< std::string > musclesNames;
    extern boost::mutex musclesNamesMutex;

    extern std::vector< std::vector < std::string > > musclesNamesOnDof;
    extern boost::mutex musclesNamesOnDofMutex;

    extern std::vector< std::string > dofNamesWithExtTorque;
    extern boost::mutex dofNamesWithExtTorqueMutex;

    extern boost::barrier readyToStart;

    extern std::vector< std::string > dofNames;
    extern Semaphore dofNamesSem;

    extern Semaphore lmtProducingDone;
    extern Semaphore emgProducingDone;
    };
};

#endif
