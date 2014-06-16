#include "ProducersFromOpenSim.h"
#include "LmtMaFromOpenSim.h"
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <stdlib.h>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include "OpenSim/OpenSim.h"
#include "OpenSim/Tools/InverseDynamicsTool.h"


class ArrayConverter{
public:
    template<typename T>
    static void toStdVector(const OpenSim::Array<T>& srcArray, std::vector<T>& dstVector) {
        dstVector.clear();
        int size = srcArray.getSize();
        dstVector.resize(size);
        for(int i = 0; i < size; ++i)
        dstVector.at(i) = srcArray.get(i);
    }
    template<typename T>
    static void fromStdVector(OpenSim::Array<T>& dstArray, const std::vector<T>& srcVector) {
        for(typename vector<T>::const_iterator it(srcVector.begin()); it != srcVector.end(); ++it)
            dstArray.append(*it);
    }


};



ProducersFromOpenSim::ProducersFromOpenSim(const std::string& osimModelFilename, 
                                           const std::string& ikMotFilename, 
                                           const vector<string>& muscleNames, 
                                           const vector<vector <string> >& musclesOnDofs, 
                                           const vector<string>& dofNames):
osimModelFilename_(osimModelFilename), ikFilename_(ikMotFilename), muscleNames_(muscleNames), musclesOnDofs_(musclesOnDofs), dofNames_(dofNames) {

    fs::path motFile(ikFilename_);
    if(!fs::exists(motFile) && !fs::is_directory(motFile)) {
        cout << ikMotFilename + " not found.\n";
        exit(EXIT_FAILURE);
    }
    startLmtMaProducers();
    
    //not used anymore.. but cool stuff
    /*
    const string ikMask("ik.mot");
    const string grfMask("grf.mot");

    fs::path inputDir(inputDirName_);
    if(!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        cout << inputDirName + " not found.\n";
        exit(EXIT_FAILURE);
    }
    fs::directory_iterator endIt;
    for(fs::directory_iterator dirIt(inputDir); dirIt != endIt; ++dirIt) {
        if(fs::is_regular_file(dirIt->status())) {
            string filename(dirIt->path().filename().string());
            if(filename.find(ikMask) != string::npos)
                ikFilename_ = filename;
            if(filename.find(grfMask) != string::npos)
                grfFilename_ = filename;						
        }
    }
    
    if(!ikFilename_.empty()) {
        startLmtMaProducers();
        if(!grfFilename_.empty())
            startTorqueProducer();
    } else {
        std::cout << "ik motion file not found in " + inputDirName << endl;
        exit(EXIT_FAILURE);
    }

    */
}

void ProducersFromOpenSim::startLmtMaProducers() {

    cout << "IK File: " << ikFilename_ << endl;
    const string motionFilename(ikFilename_);
    OpenSim::Array<std::string> osimMuscles, osimDofs;
    ArrayConverter::fromStdVector(osimMuscles, muscleNames_);
    ArrayConverter::fromStdVector(osimDofs, dofNames_);


    OpenSim::Storage *motion = new OpenSim::Storage(motionFilename);
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

    //GET MA STORAGE LIST
    OpenSim::ArrayPtrs<OpenSim::Storage> storageList;
    std::vector<OpenSim::Storage> maStorageList;
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
            maStorageList.push_back(tempSto); 
        }
    }

  //  maStorageList.back().print("sorageBeforeC.osim");
    LmtMaFromOpenSim lmtMaProducer(*lmtStorage, maStorageList);
    lmtMaProducer.setMusclesOnDofs(musclesOnDofs_);
    lmtMaProducerThread_ = boost::thread(lmtMaProducer);

}

void ProducersFromOpenSim::startTorqueProducer() {

    cout << "GRF File: " << grfFilename_ << endl;
//	OpenSim::InverseDynamicsTool idTool(osimModelFilename_);
//	idTool.setExternalLoadsFileName(grfFilename_);
//	idTool.run();


}

void ProducersFromOpenSim::join() {
	   
    lmtMaProducerThread_.join();
    
}

ProducersFromOpenSim::~ProducersFromOpenSim() {

}
