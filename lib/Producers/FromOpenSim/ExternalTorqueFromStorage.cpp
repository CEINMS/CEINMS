#include "ExternalTorqueFromStorage.h"
#include "DataFromFile.h"
#include "SyncTools.h"


#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/concept_check.hpp>
#include <cstdlib>
#include "OpenSim/Common/Storage.h"
#include "ArrayConverter.h"

#include <algorithm>

#define LOG


ExternalTorqueFromStorage::ExternalTorqueFromStorage(const string& filename)
                   :filename_(filename)
{ }

bool ExternalTorqueFromStorage::externalTorqueFileExist(const string& fileName)
{
  std::ifstream dataFile(fileName.c_str());
  bool exist = dataFile.is_open();
  dataFile.close();
  return exist;
}
  
  

void ExternalTorqueFromStorage::operator()()
{
    double myTime;  
    vector<double> endOfData;

#ifdef LOG
    cout << "\nstarting externalTorqueProduce, reading from external torque data files" << endl;
#endif

    vector<string> dofNamesFromModel;
    vector<string> externalTorqueDofNames;
    getDofNames(dofNamesFromModel);
    int noDofWithExtTorque = 0;
    if(externalTorqueFileExist(filename_)) {

        OpenSim::Storage idStorage(filename_);
        
        vector<string> columnLabels;
        ArrayConverter::toStdVector(idStorage.getColumnLabels(), columnLabels);
        const vector<string> momentNamesStorage(columnLabels.begin()+1, columnLabels.end());
        for(vector<string>::const_iterator it(dofNamesFromModel.begin());
            it != dofNamesFromModel.end(); ++it) {
            vector<string>::const_iterator fIt = std::find(momentNamesStorage.begin(), momentNamesStorage.end(), ((*it)+"_moment"));
            if(fIt != momentNamesStorage.end()) {
                externalTorqueDofNames.push_back(*it);
                torqueInStorageIdx_.push_back(std::distance(momentNamesStorage.begin(), fIt));
            }
        }
    
    //set dof names with and externat torque associated on them to a global variable
        setExternalTorqueDofNames(externalTorqueDofNames); 

        vector<double> timeColumn;
        OpenSim::Array<double> tempTimeColumn;
        idStorage.getTimeColumn(tempTimeColumn);
        ArrayConverter::toStdVector(tempTimeColumn, timeColumn);


    //END OF EXTERNAL TORQUE INIT
    #ifdef LOG  

        cout << "\nnoDofWithExtTorque " << noDofWithExtTorque << endl;
        cout << "\nend of external torque INIT";
    #endif
    
        SyncTools::Shared::readyToStart.wait();
        
        for(unsigned i = 0; i < timeColumn.size(); ++i) {      
            double time(timeColumn.at(i));
            OpenSim::Array<double> tempDataFromState;
            tempDataFromState = idStorage.getStateVector(i)->getData();
            for(unsigned d = 0; d < torqueInStorageIdx_.size(); ++d)
                updateExternalTorque(vector<double>(1, tempDataFromState.get(torqueInStorageIdx_.at(d))), time, d);
                
        }
        for(unsigned d = 0; d < torqueInStorageIdx_.size(); ++d)
            updateExternalTorque(vector<double>(), 0, d); 
    }
    else {
        
        SyncTools::Shared::readyToStart.wait();
        for(unsigned d = 0; d < externalTorqueDofNames.size(); ++d)
            updateExternalTorque(vector<double>(), 0, d); 
    }
    
#ifdef LOG  
  cout << "\nexternalTorque DONE\n";
#endif

}

