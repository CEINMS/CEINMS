//__________________________________________________________________________
// Author: Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "InputDataInterpreterOsim.h"
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

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "ArrayConverter.h"

#include "OpenSim/OpenSim.h"

#include <iostream>
using std::cout;
using std::endl;



InputDataInterpreterOsim::InputDataInterpreterOsim()
:inputDirectory_("."){

}

InputDataInterpreterOsim::InputDataInterpreterOsim( const vector< string >& muscleNames, const vector< string >& dofNames )
:muscleNames_(muscleNames), dofNames_(dofNames){

}


bool InputDataInterpreterOsim::isValid() {

    //TODO: check if the trials are valid... to implement
    return true;
}



void InputDataInterpreterOsim::setInputDirectory(const string& inputDirectory) {

    inputDirectory_ = inputDirectory;
}


void InputDataInterpreterOsim::setOsimModel ( const string& osimModelFIlename ) {
    
    osimModelFilename_ = osimModelFIlename;
}


void InputDataInterpreterOsim::setDofNames ( const vector< string >& dofNames ) {

    dofNames_ = dofNames;
}

void InputDataInterpreterOsim::setMuscleNamesOnDofs(const vector<vector<string> >& muscleNamesOnDofs) {

	muscleNamesOnDofs_ = muscleNamesOnDofs;
}


void InputDataInterpreterOsim::convert ( const string& trialID, TrialData& trial ) {

    trial.id_ = trialID;
    trial.dofNames_ = dofNames_;
    readEmgFile(trial);
    readIKMotFile(trial);
    readIDStoFile(trial);
//     isValid();
    
}

void InputDataInterpreterOsim::readEmgFile(TrialData& trial) {
     
    string emgDataFilename(getFile(inputDirectory_+"/"+trial.id_, "emg.txt" ));
    emgDataFilename = inputDirectory_+"/"+trial.id_+"/"+emgDataFilename;
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


void InputDataInterpreterOsim::readIKMotFile ( TrialData& trial ) {

    string ikFilename(getFile(inputDirectory_+"/"+trial.id_, "ik.mot" ));
    ikFilename = inputDirectory_+"/"+trial.id_+"/"+ikFilename;
    OpenSim::Storage lmtData;
    vector<OpenSim::Storage> maData;
    runMuscleAnalysis(ikFilename, lmtData, maData);
    setLmt(lmtData, trial);
    setMa(maData, trial);
}


void InputDataInterpreterOsim::readIDStoFile ( TrialData& trial ) {

    string idFilename(getFile(inputDirectory_+"/"+trial.id_, "id.sto" ));
    idFilename = inputDirectory_+"/"+trial.id_+"/"+idFilename;
    OpenSim::Storage idStorage(idFilename);
        
    vector<int> torqueInStorageIdx;
    vector<string> columnLabels;
    ArrayConverter::toStdVector(idStorage.getColumnLabels(), columnLabels);
    const vector<string> momentNamesStorage(columnLabels.begin()+1, columnLabels.end());
    for(vector<string>::const_iterator it(dofNames_.begin());
        it != dofNames_.end(); ++it) {
        vector<string>::const_iterator fIt = std::find(momentNamesStorage.begin(), momentNamesStorage.end(), ((*it)+"_moment"));
        if(fIt != momentNamesStorage.end()) {
            torqueInStorageIdx.push_back(std::distance(momentNamesStorage.begin(), fIt));
        }
    }
    

    //set dof names with and externat torque associated on them to a global variable
    vector<double> timeColumn;
    OpenSim::Array<double> tempTimeColumn;
    idStorage.getTimeColumn(tempTimeColumn);
    ArrayConverter::toStdVector(tempTimeColumn, timeColumn);


    trial.noTorqueSteps_ = timeColumn.size();
    trial.torqueTimeSteps_ = timeColumn;
	trial.torqueData_.resize(torqueInStorageIdx.size());
	trial.noDoF_ = torqueInStorageIdx.size();
    for(unsigned i = 0; i < timeColumn.size(); ++i) {      
        trial.torqueTimeSteps_.push_back(timeColumn.at(i));
        OpenSim::Array<double> tempDataFromState;
        tempDataFromState = idStorage.getStateVector(i)->getData();
        for(unsigned d = 0; d < torqueInStorageIdx.size(); ++d) 
		    trial.torqueData_.at(d).push_back(tempDataFromState.get(torqueInStorageIdx.at(d))); 
		
    }
	
}


void InputDataInterpreterOsim::runMuscleAnalysis(const string& ikfilename, OpenSim::Storage& lmtData, vector<OpenSim::Storage>& maData) {
    
    OpenSim::Array<std::string> osimMuscles, osimDofs;
    ArrayConverter::fromStdVector(osimMuscles, muscleNames_);
    ArrayConverter::fromStdVector(osimDofs, dofNames_);


    OpenSim::Storage *motion = new OpenSim::Storage(ikfilename);
    double startTime = motion->getFirstTime();
    double endTime = motion->getLastTime();
    OpenSim::Model osimModel(osimModelFilename_);
    OpenSim::MuscleAnalysis muscleTendonLengthAnalysis(&osimModel); 

    muscleTendonLengthAnalysis.setCoordinates(osimDofs);
    muscleTendonLengthAnalysis.setMuscles(osimMuscles);
    muscleTendonLengthAnalysis.setStartTime(startTime);
    muscleTendonLengthAnalysis.setEndTime(endTime); 
    muscleTendonLengthAnalysis.setComputeMoments(true);
    muscleTendonLengthAnalysis.setPrintResultFiles(false);
    osimModel.addAnalysis(&muscleTendonLengthAnalysis);

    OpenSim::AnalyzeTool musclePlotter(osimModel);

    SimTK::State& muscleState = osimModel.initSystem();
    osimModel.getMultibodySystem().realize(muscleState, SimTK::Stage::Position);
    musclePlotter.setStatesFromMotion(muscleState, *motion, true);
    musclePlotter.addAnalysisSetToModel();
    musclePlotter.setInitialTime(startTime);
    musclePlotter.setFinalTime(endTime);
    musclePlotter.run();

    //A qua

    //GET LMT STORAGE
    OpenSim::Storage *lmtStorage;
    lmtStorage = muscleTendonLengthAnalysis.getMuscleTendonLengthStorage();

    lmtData = *lmtStorage;
    
    //GET MA STORAGE LIST
    OpenSim::ArrayPtrs<OpenSim::Storage> storageList;
    storageList = muscleTendonLengthAnalysis.getStorageList();
    for(int i = 0; i < osimDofs.size(); ++i) {
        bool found = false;
        int k = 0;
        while(!found && k < storageList.size()) {
            if(storageList[k]->getName() == ("MomentArm_"+osimDofs[i]) )
                found = true;
            else
                ++k;
        };
        if(found)  {
            OpenSim::Storage tempSto(*(storageList.get(k)));
            maData.push_back(tempSto); 
        }
    }
    
}

void InputDataInterpreterOsim::setLmt ( const OpenSim::Storage& lmtData, TrialData& trial ) {

     //Safety: check if the moment arms time column is the same
    
    OpenSim::Array<double> tempTimeColumn;
    lmtData.getTimeColumn(tempTimeColumn);
    vector<double> timeColumn;
    ArrayConverter::toStdVector(tempTimeColumn, timeColumn);
    
    trial.noLmtSteps_ = timeColumn.size();
	cout << trial.noLmtSteps_ << endl;
    for(unsigned i = 0; i < timeColumn.size(); ++i) {      

        vector<double> dataFromState;
        ArrayConverter::toStdVector(lmtData.getStateVector(i)->getData(), dataFromState);
        trial.lmtTimeSteps_.push_back(timeColumn.at(i));
        trial.lmtData_.push_back(dataFromState);
    }

}


void InputDataInterpreterOsim::setMa ( const vector< OpenSim::Storage >& maData, TrialData& trial ) {

    trial.maData_.resize(maData.size());
    OpenSim::Array<double> tempTimeColumn;
    maData.front().getTimeColumn(tempTimeColumn);
    vector<double> timeColumn;
    ArrayConverter::toStdVector(tempTimeColumn, timeColumn);
    
    for(unsigned i = 0; i < timeColumn.size(); ++i) {      
        for(unsigned d = 0; d < maData.size(); ++d) {
            OpenSim::Array<double> tempMaDataFromState;
            tempMaDataFromState = maData.at(d).getStateVector(i)->getData();
            vector<double> dataFromState;
            ArrayConverter::toStdVector(tempMaDataFromState, dataFromState);
            vector<double> maDataFromState;
            stripStateData(dataFromState, maDataFromState, d);
            trial.maData_.at(d).push_back(maDataFromState);
        }
    }
}



string InputDataInterpreterOsim::getFile ( const string& aPath, const string& mask ) {

    fs::path currentPath(aPath);
    if(!fs::exists(currentPath) || !fs::is_directory(currentPath)) {
        cout << aPath + " not found.\n";
        exit(EXIT_FAILURE);
    }
    string foundFile;
    fs::directory_iterator endIt;
    for(fs::directory_iterator dirIt(currentPath); dirIt != endIt; ++dirIt) {
        if(fs::is_regular_file(dirIt->status())) {
            string filename(dirIt->path().filename().string());
            if(filename.find(mask) != string::npos)
                foundFile = filename;                    
        }
    }
    
    if(foundFile.empty()) {
        std::cout << "file found in " + aPath << endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "File: " +foundFile << std::endl;
    return foundFile;
}

void InputDataInterpreterOsim::stripStateData(const vector<double>& srcData, vector<double>& dstData, unsigned whichDof) {

    if(whichDof > muscleNamesOnDofs_.size()) {
        dstData = srcData;
        return;
    }

    for(vector<string>::const_iterator it = muscleNamesOnDofs_.at(whichDof).begin(); 
        it != muscleNamesOnDofs_.at(whichDof).end();
        ++it) {

        vector<string>::iterator position = std::find(muscleNames_.begin(), muscleNames_.end(), *it);
        if(position != muscleNames_.end()) 
            dstData.push_back(srcData.at(std::distance(muscleNames_.begin(), position)));
    }

}


void InputDataInterpreterOsim::convert ( const list< string >& trialIDs, list< TrialData >& trials ) {

    trials.clear();
    for(list<string>::const_iterator it = trialIDs.begin(); it != trialIDs.end(); ++it) {
        TrialData aTrial;
        convert(*it, aTrial);
        trials.push_back(aTrial);
    }
}



void InputDataInterpreterOsim::convert(const list<string>& trialIDs, vector<TrialData>& trials ) {

    trials.clear();
    for(list<string>::const_iterator it = trialIDs.begin(); it != trialIDs.end(); ++it) {
        TrialData aTrial;
        convert(*it, aTrial);
        trials.push_back(aTrial);
    }
}

