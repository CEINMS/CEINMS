//__________________________________________________________________________
// Author: Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef InputDataInterpreterOsim_h
#define InputDataInterpreterOsim_h
#include <list>
#include <vector>
#include <string>
#include "OpenSim/OpenSim.h"

class TrialData;

class InputDataInterpreterOsim {
  
public:
    InputDataInterpreterOsim();
    InputDataInterpreterOsim(const std::vector<std::string>& muscleNames, const std::vector<std::string>& dofNames);
    void convert(const std::list<std::string>& from, std::list<TrialData>& to); 
    void convert(const std::list<std::string>& from, std::vector<TrialData>& to); 
    void convert(const std::string& from, TrialData& to);
    void setOsimModel(const std::string& osimModelFIlename);
    void setInputDirectory(const std::string& inputDirectory);
    void setMuscleNames(const std::vector<std::string>& muscleNames);
    void setDofNames(const std::vector<std::string>& dofNames);
    void setMuscleNamesOnDofs(const std::vector< std::vector<std::string> >& muscleNamesOnDofs);
    void setEmgGeneratorXmlFilename(const std::string& filename){emgGeneratorXmlFilename_=filename;};

private:
    void setLmt(const OpenSim::Storage& from, TrialData& to);
    void setMa(const std::vector<OpenSim::Storage>& from, TrialData& to);
    void readEmgFile(TrialData& trial);
    void readIKMotFile(TrialData& trial);
    void readIDStoFile(TrialData& trial);
    void stripStateData(const std::vector<double>& srcData, std::vector<double>& dstData, unsigned whichDof);
    void runMuscleAnalysis(const std::string& ikFilename, OpenSim::Storage& lmtData, std::vector<OpenSim::Storage>& maData);
    std::string getFile(const std::string& path, const std::string& mask); 
//    void readMuscleTendonLengthFile(TrialData& trial);
//    void readMomentArmsFiles(TrialData& trial);
//    void readExternalTorqueFiles(TrialData& trial);
    bool isValid();
    std::string inputDirectory_;
    std::string osimModelFilename_;
    std::vector<std::string> muscleNames_;
    std::vector<std::string> dofNames_;
    std::vector< std::vector<std::string> > muscleNamesOnDofs_;
    std::string emgGeneratorXmlFilename_;
    
};


#endif