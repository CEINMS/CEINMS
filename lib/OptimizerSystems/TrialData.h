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

namespace CEINMS {

    class TrialData {
    public:
        std::string id;
        DataTable<double> emgData;
        DataTable<double> lmtData;
        DataTable<double> torqueData;

        size_t noDoF;
        std::vector<std::string> dofNames;
        std::vector<DataTable<double>> maData;

    };
}

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
