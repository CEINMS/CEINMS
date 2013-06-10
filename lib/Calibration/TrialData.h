#ifndef TrialData_h
#define TrialData_h

#include <vector>
#include <string>

class TrialData {
public:

  void crop( const double from, const double to);
  std::string id_; // OK
  int noMuscles_; // OK
  int noEmgSteps_; // OK
  std::vector< double > emgTimeSteps_;  // OK
  std::vector< std::vector < double > > emgData_; // OK 
  int noLmtSteps_; // OK
  std::vector< double > lmtTimeSteps_; // OK
  std::vector< std::vector < double > > lmtData_; // OK
  int noDoF_; // OK
  std::vector<std::string> dofNames_; // OK 
  // each ma has a different matrix
  std::vector< std::vector < std::vector < double > > > maData_; // OK 
  unsigned int noTorqueSteps_;
  std::vector< std::vector < double > > torqueData_;
};


#include "TrialData.cpp"

#endif
