//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef TrialData_h
#define TrialData_h

#include <vector>
#include <string>
#include <iostream>

class TrialData {
public:

 // void crop( const double from, const double to);
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
  std::vector< double > torqueTimeSteps_; // OK
  std::vector< std::vector < double > > torqueData_;
 // friend std::ostream& operator<< (std::ostream& output, const TrialData& b);
};

/*
std::ostream& operator<< (std::ostream& output, const TrialData& rhs) {

	output << "ID " << rhs.id_ << std::endl;
	output << "noMuscles " << rhs.noMuscles_ << std::endl;
    output << "noEMGsteps " << rhs.noEmgSteps_ << std::endl; 
    output << "noLmtsteps " << rhs.noLmtSteps_ << std::endl; 
	output << "noDofs " << rhs.noDoF_ << std::endl; 
	output << "noTorqueSteps " << rhs.noTorqueSteps_ << std::endl; 

	return output;
	}*/
//#include "TrialData.cpp"

#endif
