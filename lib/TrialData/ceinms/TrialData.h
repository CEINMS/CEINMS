/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): C. Pizzolato, M. Reggiani                                       *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#ifndef ceinms_TrialData_h
#define ceinms_TrialData_h

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "DataTable.h"

namespace ceinms {

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
