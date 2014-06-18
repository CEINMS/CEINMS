#include "LmtMaFromOpenSim.h"
#include "DataFromFile.h"
#include "InputQueues.h"
//#include "SyncTools.h"
//#include "Array2Vector.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iterator>
//#include <boost/thread/mutex.hpp>
//#include <boost/thread/barrier.hpp>
#include <cstdlib>

#include <algorithm>


#define LOG

void LmtMaFromOpenSim::operator()()
{
    OpenSim::Array<double> timeColumn;
    lmtData_.getTimeColumn(timeColumn);

    //Safety: check if the moment arms time column is the same
    for(auto& maIt: maData_) {
        OpenSim::Array<double> maTimeColumn;
        maIt.getTimeColumn(maTimeColumn);

        if(! (maTimeColumn == timeColumn)) {
            cout << "ERROR: Time between muscle tendon length and moment arm mismatch\nExit\n";
            exit(EXIT_FAILURE);
        }
    }
      // 1. wait - if required - for the required subscriptions to happen
    CEINMS::InputConnectors::doneWithSubscription.wait();

    for(unsigned i = 0; i < timeColumn.size(); ++i) {
        OpenSim::Array<double> tempDataFromState;
        tempDataFromState = lmtData_.getStateVector(i)->getData();
        vector<double> dataFromState;
        ArrayConverter::toStdVector<double>(tempDataFromState, dataFromState);

        vector<double> selectedLmtData(musclesNames_.size());
        for (int i=0; i < musclesNames_.size(); ++i)
            selectedLmtData.at(i) = dataFromState.at(musclePositionInLmtStorage_.at(i));

        updateLmt(selectedLmtData, timeColumn[i]);

        for (unsigned int currentDof = 0; currentDof < dofNames_.size(); ++currentDof)
        {
            tempDataFromState = maData_.at(currentDof).getStateVector(i)->getData();
            dataFromState.clear();
            ArrayConverter::toStdVector<double>(tempDataFromState, dataFromState);

            vector<double> selectedMaData(musclePositionsInMaStorages_.at(currentDof).size());
            for (int i = 0; i < selectedMaData.size(); ++i)
                selectedMaData.at(i) = dataFromState.at(musclePositionsInMaStorages_.at(currentDof).at(i));

            updateMomentArms(selectedMaData, timeColumn[i], currentDof);
        }
    }

  vector<double> endOfData;
  updateLmt(endOfData, 0);
  for (unsigned int i = 0; i < dofNames_.size(); ++i)
    updateMomentArms(endOfData, 0, i);

#ifdef LOG
  cout << "\nLmtMa: lmtMa DONE\n";
#endif
  
}

