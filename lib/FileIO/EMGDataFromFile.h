//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef EMGDataFromFile_h
#define EMGDataFromFile_h

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

template <typename EMGgenerator>
class EMGDataFromFile {
public:
  EMGDataFromFile() { std::cout << "You should not be there\n"; }
  EMGDataFromFile(const std::string& inputFile, const std::string& emgGeneratorFile);
  const std::vector<std::string>& getMusclesNames() const { return EMGgenerator_.getMusclesNames(); }
  void getMusclesNames( std::vector< std::string >& muscleNames ) const {muscleNames = EMGgenerator_.getMusclesNames(); }
  void readNextEmgData();
  inline double getCurrentTime() const {return currentDataTime_;}
  bool areStillData() const { return currentTimeStep_ < noTimeSteps_; } 
  const std::vector<double>& getCurrentData() const {return currentEMGData_;}
  int getNoTimeSteps() const {return noTimeSteps_;}
  ~EMGDataFromFile();
private:
  std::ifstream EMGDataFile_;
  int noMuscles_;
  int noTimeSteps_;
  int currentTimeStep_;
  double currentDataTime_;
  std::vector<std::string> muscleNames_;
  std::vector<double> currentReadEMG_;
  std::vector<double> currentEMGData_;
  EMGgenerator EMGgenerator_;
};

//#include "EMGDataFromFile.cpp"

#endif

