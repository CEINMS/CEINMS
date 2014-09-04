//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <sstream>
using std::stringstream;
#include <stdlib.h>

#include "EMGDataFromFile.h"

template <typename EMGgenerator>
EMGDataFromFile<EMGgenerator>::EMGDataFromFile(const string& EMGDataFilename, const std::string& emgGeneratorFile)
:EMGDataFile_(EMGDataFilename.c_str()), EMGgenerator_(emgGeneratorFile) {
 
  if (!EMGDataFile_.is_open()) {
    cout << "ERROR emg file: " << EMGDataFilename << " could not be open\n";
    exit(EXIT_FAILURE);
  }

  string line;
  getline(EMGDataFile_, line, '\n');
  while (line!="endheader" && line!= "endheader\r" && !EMGDataFile_.eof())
  {
      if( size_t foundInfo=line.find("nRows=")!=string::npos)
      {
          noTimeSteps_=atoi(line.substr(6).c_str());
      }
      if( size_t foundInfo=line.find("datarows ")!=string::npos)
      {
          noTimeSteps_=atoi(line.substr(9).c_str());
      }
      if( size_t foundInfo=line.find("nColumns=")!=string::npos)
      {
          noMuscles_=atoi(line.substr(9).c_str())-1; //nColumns includes the "time" column
      }
      if( size_t foundInfo=line.find("datacolumns ")!=string::npos)
      {
          noMuscles_=atoi(line.substr(12).c_str())-1; //nColumns includes the "time" column
      }
      getline((EMGDataFile_), line, '\n');
  }
  // reading muscles
  getline((EMGDataFile_), line, '\n');  
  stringstream myStream(line);
  string nextColumnName;
    // the first is the "Time"
  string timeName;
  myStream >> timeName;
    // then we have their names
  int noReadColumns = 0;
  do {
      myStream >> nextColumnName;
      muscleNames_.push_back(nextColumnName);
  } while (!myStream.eof() && ++noReadColumns < noMuscles_);


  if (noMuscles_ != muscleNames_.size()) {
    cout << "Something is wrong. " << noMuscles_ << " muscles should be in the file "
         << "and we have : " << muscleNames_.size();
    exit(EXIT_FAILURE);
  } 
  
  if ( !EMGgenerator_.checkInputSignalsNames(muscleNames_) ) {
	  vector<string> musclesNamesFromGenerator;
	  EMGgenerator_.getInputSignalsNames(musclesNamesFromGenerator);
	  std::cout << " generator names - file names\n";
		for(unsigned i = 0; i < musclesNamesFromGenerator.size() && i < muscleNames_.size() ; ++i) {
			string genName(musclesNamesFromGenerator.at(i)), fileName(muscleNames_.at(i)), separator("  -  ");
			if(genName != fileName) separator = "  <>  ";
			std::cout << genName << separator <<  fileName << std::endl;
		}
    cout << "THE EMG generator is not able to generate EMG for your muscles starting from muscle in the file! (muscle names are different)\n";

    exit(EXIT_FAILURE);
  }
  
//TODO fix resize
  currentReadEMG_.resize(EMGgenerator_.getNoInputSignals());
  currentEMGData_.resize(EMGgenerator_.getNoExcitations());
  currentDataTime_ = 0.;
  currentTimeStep_ = 0;
}

template <typename EMGgenerator>
void EMGDataFromFile<EMGgenerator>::readNextEmgData()  {

  // first we build a vector of emg read from the file
 
  string line;
  getline(EMGDataFile_, line, '\n');
  stringstream myStream(line);
  double value;
  currentReadEMG_.clear();
  myStream >>  currentDataTime_;
 // cout << "EMGdatafromfile in emg.txt: time step "<< currentDataTime_ << endl;
  int noReadMuscles=0;
  do {
    myStream >> value;
  //  cout << "EMGdatafromfile in emg.txt: value "<< value << endl;
    currentReadEMG_.push_back(value);
  } while (!myStream.eof() && ++noReadMuscles<noMuscles_);
 // cout << "EMGDataFromFile: just read " << currentReadEMG_.size() << " values\n"; 
  EMGgenerator_.convert(currentReadEMG_, currentEMGData_);
 
/*
  int noFromMuscles = EMGgenerator_.getNoFromMuscles();
  // 1. first their names
  for (int i = 0; i < noFromMuscles; ++i) {
    double value;
    EMGDataFile_ >> value; 
    currentReadEMG_[i] = value;
  } 

  // then we convert them on what they need
  EMGgenerator_.convert(currentReadEMG_, currentEMGData_);
*/
  ++currentTimeStep_;

}

template <typename EMGgenerator>
EMGDataFromFile<EMGgenerator>::~EMGDataFromFile() {
  EMGDataFile_.close();
}


#include "policyTemplates.h"

