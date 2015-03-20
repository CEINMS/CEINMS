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
#include <map>
#include <string>
#include <iostream>
#include "DataTable.h"



class TrialData {
public:
    std::string id_;
    DataTable<double> emgData_;
    DataTable<double> lmtData_;
    DataTable<double> torqueData_;

    size_t noDoF_;
    std::vector<std::string> dofNames_;
    std::vector<DataTable<double>> maData_;

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
