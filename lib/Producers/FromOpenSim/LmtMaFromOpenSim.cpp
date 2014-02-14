#include "LmtMaFromOpenSim.h"
#include "DataFromFile.h"
#include "SyncTools.h"
#include "Array2Vector.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iterator>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <cstdlib>

#include <algorithm>


#define LOG

LmtMaFromOpenSim::LmtMaFromOpenSim(const OpenSim::Storage& lmtData, const vector<OpenSim::Storage>& maData)
    :lmtData_(lmtData), maData_(maData)
{ 

//	maData_.back().print("sorageAfterC.osim");
}


void LmtMaFromOpenSim::stripStateData(const vector<double>& srcData, vector<double>& dstData, unsigned whichDof) {

    if(whichDof > musclesOnDofs_.size()) {
        dstData = srcData;
        return;
    }

    for(vector<string>::const_iterator it = musclesOnDofs_.at(whichDof).begin(); 
        it != musclesOnDofs_.at(whichDof).end();
        ++it) {

        vector<string>::iterator position = std::find(muscleNames_.begin(), muscleNames_.end(), *it);
        if(position != muscleNames_.end()) 
            dstData.push_back(srcData.at(std::distance(muscleNames_.begin(), position)));
    }

}

void LmtMaFromOpenSim::operator()()
{

    vector<double> endOfData;

    cout << "- Start: Reading muscle-tendon legths and moment arms from file" << endl;

    vector<string> dofNamesFromModel;
    getDofNames(dofNamesFromModel);
    int noDof = dofNamesFromModel.size();
//LMT INIT
    vector<string> tempLabels, lmtMusclesNames;
    OpenSim::Array<string> lmtLabels = lmtData_.getColumnLabels();
    Array2Vector::convert(lmtLabels, tempLabels);
    lmtMusclesNames.assign(tempLabels.begin()+1, tempLabels.end());

    //lmt muscles name are set to global variable
    setLmtMusclesNames(lmtMusclesNames);
    setMomentArmsMusclesNames(musclesOnDofs_);
    
//MOMENT ARMS INIT

//END OF MOMENT ARMS INIT
//    if(!musclesOnDofs_.empty())
//        setNoDof(musclesOnDofs_.size());
//    else
//        setNoDof(maData_.size());


    SyncTools::Shared::readyToStart.wait();

    OpenSim::Array<double> tempTimeColumn;
    lmtData_.getTimeColumn(tempTimeColumn);
    vector<double> timeColumn;
    Array2Vector::convert(tempTimeColumn, timeColumn);

    //Safety: check if the moment arms time column is the same
    for(vector<OpenSim::Storage>::const_iterator maIt = maData_.begin(); maIt != maData_.end(); ++maIt) {
        maIt->getTimeColumn(tempTimeColumn);
        vector<double> maTimeColumn;
        Array2Vector::convert(tempTimeColumn, maTimeColumn);
        if(maTimeColumn != timeColumn) {
            cout << "ERROR: Time between muscle tendon length and moment arm mismatch\nExit\n";
            exit(EXIT_FAILURE);
        }
    }
      
    OpenSim::Array<string> columnLabels = lmtData_.getColumnLabels();
    vector<string> convertedLabels;
    Array2Vector::convert(columnLabels, convertedLabels);
    muscleNames_.assign(convertedLabels.begin()+1, convertedLabels.end());

    for(unsigned i = 0; i < timeColumn.size(); ++i) {      

        OpenSim::Array<double> tempDataFromState;
        tempDataFromState = lmtData_.getStateVector(i)->getData();
        vector<double> dataFromState;
        Array2Vector::convert(tempDataFromState, dataFromState);
        updateLmt(dataFromState, timeColumn.at(i));

        for(unsigned d = 0; d < maData_.size(); ++d) {
            OpenSim::Array<double> tempMaDataFromState;
            tempMaDataFromState = maData_.at(d).getStateVector(i)->getData();
            dataFromState.clear();
            Array2Vector::convert(tempMaDataFromState, dataFromState);
            vector<double> maDataFromState;
            stripStateData(dataFromState, maDataFromState, d);
            updateMomentArms(maDataFromState, timeColumn.at(i), d);
        }
    }

    updateLmt(endOfData, 0);
    for (unsigned int i = 0; i < maData_.size(); ++i)
        updateMomentArms(endOfData, 0, i); 

	SyncTools::Shared::lmtProducingDone.notify(); //used for validate curve only
    cout << "-- Done: Reading muscle-tendon legths and moment arms from file" << endl;

  
}

