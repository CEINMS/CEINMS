#include "ExternalTorquesFromStorage.h"
#include "DataFromFile.h"

#include "InputQueues.h"
#include "Utilities.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include <cstdlib>
#include "OpenSim/Common/Storage.h"
#include "ArrayConverter.h"

#include <algorithm>

#define LOG

bool ExternalTorquesFromStorage::externalTorqueFileExist(const string& fileName)
{
  std::ifstream dataFile(fileName.c_str());
  bool exist = dataFile.is_open();
  dataFile.close();
  return exist;
}



void ExternalTorquesFromStorage::operator()()
{
    if (CEINMS::InputConnectors::externalTorquesAvailable)  {
    #ifdef LOG
        std::cout << "\n ExtTorque: external Torques available " << std::endl;
    #endif
        OpenSim::Storage idStorage(filename_);

        vector<string> columnNamesInStorageFile;
        ArrayConverter::toStdVector(idStorage.getColumnLabels(), columnNamesInStorageFile);
        std::vector<std::string> torqueNamesToFind;
        for(auto& it: dofNames_)
            torqueNamesToFind.push_back(it+"_moment");

        dofPosInStorage_ = findMapping(torqueNamesToFind, columnNamesInStorageFile);

        OpenSim::Array<double> timeColumn;
        idStorage.getTimeColumn(timeColumn);

        CEINMS::InputConnectors::doneWithSubscription.wait(); //TODO: check if it's right that this is inside the if() block

        //
        for(unsigned t = 0; t < timeColumn.size(); ++t)
        {
            double time(timeColumn[t]);
            OpenSim::Array<double>newTorquesData;
            newTorquesData = idStorage.getStateVector(t)->getData();
            vector<double> selectedTorquesData(dofNames_.size());
            for (int i = 0; i < dofNames_.size(); ++i)
                selectedTorquesData.at(i) = newTorquesData[dofPosInStorage_.at(i)];

            updateExternalTorques(selectedTorquesData, time);
        }

        vector<double> endOfTorques;
        updateExternalTorques(endOfTorques, std::numeric_limits<double>::infinity());

    }
    else {
#ifdef LOG
        cout << "\n ExtTorque: external Torques not available " << endl;
#endif
    }

#ifdef LOG
    cout << "\nexternalTorque DONE\n";
#endif
}
