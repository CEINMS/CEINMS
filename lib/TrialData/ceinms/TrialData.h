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
        TrialData();
        TrialData(
            const std::vector<std::string>& dofNames,
            const std::vector<std::string>& muscleNames,
            const std::string& id);
        TrialData(
            const DataTable<double>& emgData, 
            const DataTable<double>& lmtData, 
            const std::vector<DataTable<double>>& maData, 
            const DataTable<double>& torqueData, 
            const std::vector<std::string>& dofNames, 
            const std::vector<std::string>& muscleNames, 
            const std::string& id);
        void setProperties(
            const std::vector<std::string>& dofNames,
            const std::vector<std::string>& muscleNames,
            const std::string& id);
        void setEmgData(const DataTable<double>& emgData);
        void setLmtMaData(const DataTable<double>& emgData, const std::vector<DataTable<double>>& maData);
        void setTorqueData(const DataTable<double>& torqueData);
        void setOpenSimData(
            const std::string& openSimModelFile, 
            const std::string& externalLoadsFile, 
            const std::string& motionFile);
        bool hasEmgData() const { return hasEmgData_; }
        bool hasLmtMaData() const { return hasLmtMaData_; }
        bool hasTorqueData() const { return hasTorqueData_; }
        bool hasOpenSimData() const { return hasOpenSimData_; }
        bool isConsistent() {
            bool consistent =
                emgData_.getNRows() > 0 &&
                lmtData_.getNRows() > 0 &&
                torqueData_.getNRows() > 0;
            return consistent;
        }

        std::vector<std::string> getDofNames() const { return dofNames_; }
        std::vector<std::string> getMuscleNames() const { return muscleNames_; }
        std::size_t getNoDofs() const { return noDoF_; }
        std::size_t getNoMuscles() const { return muscleNames_.size(); }
        std::string getId() const { return id_; }
        const DataTable<double>& getEmgData() const { return emgData_; }
        const DataTable<double>& getLmtData() const { return lmtData_; }
        const DataTable<double>& getTorqueData() const { return torqueData_; }
        const std::vector<DataTable<double>>& getMaData() const { return maData_; }
        const DataTable<double>& getQ() const { return q_; }
        const DataTable<double>& getQd() const { return qd_; }
        const DataTable<double>& getQdd() const { return qdd_; }
        std::string getOpenSimModelFile() const { return openSimModelFile_; }
        std::string getExternalLoadFile() const { return externalLoadsFile_; }
        void crop(double startTime, double stopTime);
    private:
        std::string id_;
        std::vector<std::string> dofNames_, muscleNames_;
        size_t noDoF_;
        DataTable<double> emgData_;
        DataTable<double> lmtData_;
        DataTable<double> torqueData_;
        std::vector<DataTable<double>> maData_;

        void updateKinematicData(const std::string& motionFile, double fc = 8);
        std::string openSimModelFile_;
        std::string externalLoadsFile_;
        DataTable<double> q_, qd_, qdd_;

        bool hasEmgData_;
        bool hasLmtMaData_;
        bool hasTorqueData_;
        bool hasOpenSimData_;

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
