// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com

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
  EMGDataFromFile(const std::string& inputFile);
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

