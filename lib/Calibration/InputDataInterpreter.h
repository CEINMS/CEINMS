//__________________________________________________________________________
// Author: Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef InputDataInterpreter_h
#define InputDataInterpreter_h
#include <list>
#include <vector>
#include <string>

class TrialData;

class InputDataInterpreter {
  
public:
    InputDataInterpreter();
    InputDataInterpreter(const std::vector<std::string>& muscleNames, const std::vector<std::string>& dofNames); //TODO: should add the muscles for each dof's moment arms
    void convert(const std::list<std::string>& from, std::list<TrialData>& to); 
    void convert(const std::list<std::string>& from, std::vector<TrialData>& to); 
    void convert(const std::string& from, TrialData& to);
    void setInputDirectory(const std::string& inputDirectory);
    void setMuscleNames(const std::vector<std::string> muscleNames);
    void setDofNames(const std::vector<std::string> dofNames);
private:
    void readEmgFile(TrialData& trial);
    void readMuscleTendonLengthFile(TrialData& trial);
    void readMomentArmsFiles(TrialData& trial);
    void readExternalTorqueFiles(TrialData& trial);
    bool checkOrSetMuscleNames(const std::vector <std::string>& muscleNames, std::vector<size_t>& muscleIndices); 
    bool isValid();
    std::string inputDirectory_;
    std::vector<std::string> muscleNames_;
    std::vector<std::string> dofNames_;
    
};


#endif