// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com

#ifndef DataFromFile_h
#define DataFromFile_h

#include <fstream>
#include <iostream>
#include <vector>


/**
 * \brief This class read from a file either lmt or ma data
 * You should have a file that includes 
 * write here the structure of the file
 */
class DataFromFile {
public:
  DataFromFile() { std::cout << "You should not be there\n"; }
  DataFromFile(const std::string& inputFile);
  const std::vector<std::string>& getMusclesNames() const { return muscleNames_; }
  void readNextData();
  int getNoTimeSteps() const {return noTimeSteps_;}
  bool areStillData() const { return currentTimeStep_ < noTimeSteps_; } 
  inline double getCurrentTime() const {return currentDataTime_;}
  const std::vector<double>& getCurrentData() const;
  ~DataFromFile();
private:
  DataFromFile(const DataFromFile& orig) {};
  DataFromFile& operator=(const DataFromFile& orig) {};
  std::string dataFileName_;
  std::ifstream dataFile_;
  unsigned int noMuscles_;
  std::vector<std::string> muscleNames_;
  int currentTimeStep_;
  int noTimeSteps_;
  double currentDataTime_;
  std::vector<double> currentData_;
};



#endif
