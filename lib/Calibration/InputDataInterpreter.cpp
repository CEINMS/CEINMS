//__________________________________________________________________________
// Author: Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "InputDataInterpreter.h"
#include "TrialData.h"
#include "EMGDataFromFile.h"
#include "EMGgenerator/EMGgeneratorFrom16To34.h"
#include "EMGgenerator/EMGgeneratorFromXml.h"
#include "DataFromFile.h"
#include "DataFromStorageFile.h"
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <list>
#include <boost/concept_check.hpp>
using std::list;

InputDataInterpreter::InputDataInterpreter() //should be disabled??
:inputDirectory_("."){

}

InputDataInterpreter::InputDataInterpreter ( const vector< string >& muscleNames, const vector< string >& dofNames )
:muscleNames_(muscleNames), dofNames_(dofNames){

}


bool InputDataInterpreter::isValid() {

    //TODO: check if the trials are valid... to implement
    return true;
}

bool InputDataInterpreter::checkOrSetMuscleNames(const vector< string >& muscleNames, vector< size_t >& muscleIndices)
{
    if (muscleNames_.empty()) // no muscles set yet
    {
        muscleNames_=muscleNames;
        muscleIndices.resize(muscleNames.size());
        for (size_t i=0; i< muscleNames_.size(); ++i)
            muscleIndices.at(i)=i;
        return true;
    }
    else
    {
        bool ok=true;
        muscleIndices.resize(muscleNames_.size());
        vector<size_t>::iterator indIt = muscleIndices.begin();
        for (vector<string>::const_iterator muscIt = muscleNames.begin(); muscIt!=muscleNames.end(); ++muscIt, ++indIt)
        {
            vector<string>::const_iterator foundIt = std::find(muscleNames_.begin(), muscleNames_.end(), *muscIt);
            if ( foundIt!=muscleNames_.end())
                *indIt = foundIt-muscleNames_.begin();
            else ok=false;
        }
        return ok;
    }

}

void InputDataInterpreter::setInputDirectory(const string& inputDirectory) {

    inputDirectory_ = inputDirectory;
}


void InputDataInterpreter::convert ( const string& trialID, TrialData& trial ) {

    trial.id_ = trialID;
    trial.dofNames_ = dofNames_;
    readEmgFile(trial);
    readMuscleTendonLengthFile(trial);
    readMomentArmsFiles(trial);
    readExternalTorqueFiles(trial);
    trial.muscleNames_ = muscleNames_;
    trial.noMuscles_ = muscleNames_.size();
//     isValid();
    
}

void InputDataInterpreter::readEmgFile(TrialData& trial) {
     
    string emgDataFilename = inputDirectory_ + "/" + trial.id_ + "/emg.txt";
    EMGDataFromFile<EMGgeneratorFromXml> emgDataFromFile(emgDataFilename);
    vector<size_t> muscleIndices;
    if (!checkOrSetMuscleNames(emgDataFromFile.getMusclesNames(), muscleIndices))
    {
        std::cout << "Error while reading " << emgDataFilename << ": muscle names not consistent with subject.xml" <<std::endl;
        exit(EXIT_FAILURE);
    }
    trial.noEmgSteps_ = emgDataFromFile.getNoTimeSteps();
    vector<double> nextEmgData;
    nextEmgData.resize(muscleIndices.size());
  
    while(emgDataFromFile.areStillData()) {
        double EMGDataTime;
        emgDataFromFile.readNextEmgData();
        EMGDataTime = emgDataFromFile.getCurrentTime();
        trial.emgTimeSteps_.push_back(EMGDataTime);
        vector <double> dataRead = emgDataFromFile.getCurrentData();
        vector<double>::const_iterator dataReadIt=dataRead.begin();
        for (vector<size_t>::const_iterator indIt=muscleIndices.begin(); indIt!=muscleIndices.end(); ++indIt, ++dataReadIt)
        {
            nextEmgData.at(*indIt)=*dataReadIt;
        }
        trial.emgData_.push_back(nextEmgData);
    }     
}


void InputDataInterpreter::readMuscleTendonLengthFile(TrialData& trial) {

    string lmtDataFilename = inputDirectory_ + "/" + trial.id_ + "/lmt.sto";
    DataFromStorageFile lmtDataFromFile(lmtDataFilename);
    trial.noLmtSteps_ =  lmtDataFromFile.getNoTimeSteps();
    vector<size_t> muscleIndices;
    if (!checkOrSetMuscleNames(lmtDataFromFile.getColumnNames(), muscleIndices))
    {
        std::cout << "Error while reading " << lmtDataFilename << ": muscle names not consistent with subject.xml" <<std::endl;
        exit(EXIT_FAILURE);
    }
    double startLmtTime = 0;
    vector<double> nextLmtData(muscleIndices.size());
    for (int j = 0; j < trial.noLmtSteps_; ++j) {
        lmtDataFromFile.readNextData();
        double lmtDataTime(lmtDataFromFile.getCurrentTime());
        trial.lmtTimeSteps_.push_back(lmtDataTime);

        vector <double> dataRead(lmtDataFromFile.getCurrentData());
        vector<double>::const_iterator dataReadIt=dataRead.begin();
        for (vector<size_t>::const_iterator indIt=muscleIndices.begin(); indIt!=muscleIndices.end(); ++indIt, ++dataReadIt)
        {
            nextLmtData.at(*indIt)=*dataReadIt;
        }
        trial.lmtData_.push_back(nextLmtData);
    }
}


void InputDataInterpreter::readMomentArmsFiles(TrialData& trial) {

    trial.noDoF_ = dofNames_.size();
    trial.maData_.resize(dofNames_.size());
   
    for (int k = 0; k < trial.noDoF_; ++k ) {
        string maDataFilename = inputDirectory_ + "/" + trial.id_ + "/" + dofNames_.at(k) + "Ma.sto";
        DataFromStorageFile maDataFromFile(maDataFilename);
        vector <string> muscleNames= maDataFromFile.getColumnNames();
        trial.maMusclesNames_[dofNames_.at(k)] = muscleNames; //TODO should check against some input list, if only it was provided....

        std::vector<int> dofIndices(maDataFromFile.getColumnNames().size(), -1);
        for (vector<string>::iterator muscIt=muscleNames.begin(); muscIt!=muscleNames.end(); ++ muscIt)
        {

            vector<string>::const_iterator foundMusc=std::find(muscleNames_.begin(), muscleNames_.end(), *muscIt);
            if (foundMusc==muscleNames_.end())
            {
                std::cout << "ERROR while checking muscles available in "<< maDataFilename<<std::endl;
                exit(EXIT_FAILURE);
            }
        }

        for (int j = 0; j < trial.noLmtSteps_; ++j) {
            maDataFromFile.readNextData();
            vector<double> nextMaData(maDataFromFile.getCurrentData());
            trial.maData_.at(k).push_back(nextMaData);
        }
    } 
}


void InputDataInterpreter::readExternalTorqueFiles (TrialData& trial) {

    trial.noDoF_ = dofNames_.size();
    trial.torqueData_.resize(dofNames_.size());
    string torqueDataFilename = inputDirectory_ + "/" + trial.id_ + "/" + "Torque.sto";
    DataFromStorageFile torqueDataFromFile(torqueDataFilename);
    vector<string> torqueDofNames=torqueDataFromFile.getColumnNames();

    std::vector<int> dofIndices(torqueDofNames.size(), -1);
    for (vector<string>::iterator dofIt=dofNames_.begin(); dofIt!=dofNames_.end(); ++ dofIt)
    {
        vector<string>::const_iterator foundDof=std::find(torqueDofNames.begin(), torqueDofNames.end(), *dofIt + "_moment");
        if (foundDof!=torqueDofNames.end())
                dofIndices[foundDof-torqueDofNames.begin()]=dofIt-dofNames_.begin();
            else
            {
                std::cout << "ERROR while checking DOFs available in torque data file"<<std::endl;
                exit(EXIT_FAILURE);
            }
    }

        trial.noTorqueSteps_ =  torqueDataFromFile.getNoTimeSteps();

        for (int j = 0; j < trial.noTorqueSteps_; ++j) {
            torqueDataFromFile.readNextData();
            double torqueDataTime(torqueDataFromFile.getCurrentTime());
            vector<double> nextTorqueData(torqueDataFromFile.getCurrentData());
            trial.torqueTimeSteps_.push_back(torqueDataTime);
            for(size_t k=0; k<nextTorqueData.size(); ++k)
            {
                if (dofIndices.at(k) !=-1)
                    trial.torqueData_.at(dofIndices.at(k)).push_back(nextTorqueData.at(k));
            }
        }     
}
       



void InputDataInterpreter::convert ( const list< string >& trialIDs, list< TrialData >& trials ) {

    trials.clear();
    for(list<string>::const_iterator it = trialIDs.begin(); it != trialIDs.end(); ++it) {
        TrialData aTrial;
        convert(*it, aTrial);
        trials.push_back(aTrial);
    }
}



void InputDataInterpreter::convert(const list<string>& trialIDs, vector<TrialData>& trials ) {

    trials.clear();
    for(list<string>::const_iterator it = trialIDs.begin(); it != trialIDs.end(); ++it) {
        TrialData aTrial;
        convert(*it, aTrial);
        trials.push_back(aTrial);
    }
}

