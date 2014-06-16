//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "SyncTools.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition_variable.hpp>
#include "Semaphore.h"

#include <string>
using std::string;
#include <list>
using std::list;
#include <vector>
using std::vector;


namespace SyncTools {
    namespace Shared {
    float globalTimeLimit = 100.;
    const int queueBuffer = 10000;
    const unsigned int numberOfThreads = 4; // 4 se c'è il produttore della torque esterna, 3 altrimenti


    list< vector<double> > queueEmg;
    boost::mutex queueEmgMutex;
    Semaphore queueEmgSemFull(0), queueEmgSemEmpty(queueBuffer);

    list< vector<double> > queueLmt;
    boost::mutex queueLmtMutex;
    Semaphore queueLmtSemFull(0), queueLmtSemEmpty(queueBuffer);

    vector< list< vector<double> > > queueMomentArms;
    boost::mutex queueMomentArmsMutex;
    Semaphore queueMomentArmsSemFull(0), queueMomentArmsSemEmpty(queueBuffer);

    vector< list< vector <double> > > queueExternalTorque;
    boost::mutex queueExternalTorqueMutex;
    Semaphore queueExternalTorqueSemFull(0), queueExternalTorqueSemEmpty(queueBuffer);


    //EMGgenerator provides the names, emg and lmt files must have corrisponding headers
    vector<string> musclesNames;
    boost::mutex musclesNamesMutex;

    //XML model provides a list of muscles names for each dof, corrisponding moment arms files must have the same set of muscles names
    vector< vector <string> > musclesNamesOnDof;
    boost::mutex musclesNamesOnDofMutex;

    vector<string> dofNamesWithExtTorque;
    boost::mutex dofNamesWithExtTorqueMutex;

    vector<string> dofNames;
    Semaphore dofNamesSem(0);

    Semaphore lmtProducingDone(0);

    Semaphore emgProducingDone(0);

    boost::barrier readyToStart(numberOfThreads);

    };
};
