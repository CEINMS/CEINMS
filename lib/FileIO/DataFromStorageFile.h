//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef DataFromStorageFile_h
#define DataFromStorageFile_h

#include <fstream>
#include <iostream>
#include <vector>

/**
 * \brief This class read from a file either lmt or ma data
 * You should have a file that includes 
 * write here the structure of the file
 */
class DataFromStorageFile {
public:
  DataFromStorageFile() { std::cout << "You should not be there\n"; }
  DataFromStorageFile(const std::string& inputFile);
  const std::vector<std::string>& getMusclesNames() const { return muscleNames_; }
  void readNextData();
  int getNoTimeSteps() const {return noTimeSteps_;}
  bool areStillData() const { return currentTimeStep_ < noTimeSteps_; } 
  inline double getCurrentTime() const {return currentDataTime_;}
  const std::vector<double>& getCurrentData() const;
  ~DataFromStorageFile();
private:
  DataFromStorageFile(const DataFromStorageFile& orig) {};
  DataFromStorageFile& operator=(const DataFromStorageFile& orig) {};
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
