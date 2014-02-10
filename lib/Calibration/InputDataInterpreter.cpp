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
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <list>
#include <boost/concept_check.hpp>
using std::list;

InputDataInterpreter::InputDataInterpreter()
:inputDirectory_("."){

}

InputDataInterpreter::InputDataInterpreter ( const vector< string >& muscleNames, const vector< string >& dofNames )
:muscleNames_(muscleNames), dofNames_(dofNames){

}


bool InputDataInterpreter::isValid() {

    //TODO: check if the trials are valid... to implement
    return true;
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
//     isValid();
    
}

void InputDataInterpreter::readEmgFile(TrialData& trial) {
     
    string emgDataFilename = inputDirectory_ + "/" + trial.id_ + "/emg.txt";
    EMGDataFromFile<EMGgeneratorFromXml> emgDataFromFile(emgDataFilename);
    trial.noMuscles_ = emgDataFromFile.getMusclesNames().size();
    trial.noEmgSteps_ = emgDataFromFile.getNoTimeSteps();
       
    vector<double> nextEmgData;
    nextEmgData.resize(trial.noMuscles_);
  
    while(emgDataFromFile.areStillData()) {
        double EMGDataTime;
        emgDataFromFile.readNextEmgData();
        EMGDataTime = emgDataFromFile.getCurrentTime();
        trial.emgTimeSteps_.push_back(EMGDataTime);
        trial.emgData_.push_back(emgDataFromFile.getCurrentData());
    }     
}


void InputDataInterpreter::readMuscleTendonLengthFile(TrialData& trial) {

    string lmtDataFilename = inputDirectory_ + "/" + trial.id_ + "/lmt.txt";
    DataFromFile lmtDataFromFile(lmtDataFilename);
    trial.noLmtSteps_ =  lmtDataFromFile.getNoTimeSteps();   
    double startLmtTime = 0;
    for (int j = 0; j < trial.noLmtSteps_; ++j) {
        lmtDataFromFile.readNextData();
        double lmtDataTime = lmtDataFromFile.getCurrentTime();
        vector<double> nextLmtData = lmtDataFromFile.getCurrentData();
        trial.lmtTimeSteps_.push_back(lmtDataTime);
        trial.lmtData_.push_back(nextLmtData);
    }
}


void InputDataInterpreter::readMomentArmsFiles(TrialData& trial) {

    trial.noDoF_ = dofNames_.size();
    trial.maData_.resize(dofNames_.size());
   
    for (int k = 0; k < trial.noDoF_; ++k ) {
        string maDataFilename = inputDirectory_ + "/" + trial.id_ + "/" + dofNames_.at(k) + "Ma.txt";
        DataFromFile maDataFromFile(maDataFilename);
        for (int j = 0; j < trial.noLmtSteps_; ++j) {
            maDataFromFile.readNextData();
            vector<double> nextMaData = maDataFromFile.getCurrentData();
            trial.maData_.at(k).push_back(nextMaData);
        }
    } 
}


void InputDataInterpreter::readExternalTorqueFiles (TrialData& trial) {

    trial.noDoF_ = dofNames_.size();
    trial.torqueData_.resize(dofNames_.size());

    for (int k = 0; k < trial.noDoF_; ++k ) {
        string torqueDataFilename = inputDirectory_ + "/" + trial.id_ + "/" + dofNames_.at(k) + "Torque.txt";
        DataFromFile torqueDataFromFile(torqueDataFilename);
        trial.noTorqueSteps_ =  torqueDataFromFile.getNoTimeSteps();
        for (int j = 0; j < trial.noTorqueSteps_; ++j) {
            torqueDataFromFile.readNextData();
            double torqueDataTime = torqueDataFromFile.getCurrentTime();       
            vector<double> nextTorqueData = torqueDataFromFile.getCurrentData();
            trial.torqueTimeSteps_.push_back(torqueDataTime);
            trial.torqueData_.at(k).push_back(nextTorqueData.front());
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

