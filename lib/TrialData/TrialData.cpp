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

#include "ceinms/TrialData.h"
#include "ceinms/ArrayConverter.h"
#include <OpenSim/OpenSim.h>
using std::string;
using std::vector;

namespace ceinms {

    TrialData::TrialData() :
        hasEmgData_(false),
        hasLmtMaData_(false),
        hasTorqueData_(false),
        hasOpenSimData_(false)
        {}

    TrialData::TrialData(
        const std::vector<std::string>& dofNames,
        const std::vector<std::string>& muscleNames,
        const std::string& id):
        TrialData()
    {
        setProperties(dofNames, muscleNames, id);
    }

    TrialData::TrialData(
        const DataTable<double>& emgData,
        const DataTable<double>& lmtData,
        const vector<DataTable<double>>& maData,
        const DataTable<double>& torqueData,
        const vector<string>& dofNames,
        const vector<string>& muscleNames,
        const string& id) :
        TrialData(dofNames, muscleNames, id)
    {  
    
        setEmgData(emgData);
        setLmtMaData(lmtData, maData);
        setTorqueData(torqueData);
        
    }

    void TrialData::setProperties(
        const std::vector<std::string>& dofNames,
        const std::vector<std::string>& muscleNames,
        const std::string& id) {

        dofNames_ = dofNames;
        muscleNames_ = muscleNames;
        id_ = id;
        noDoF_ = dofNames.size();
    }


    void TrialData::setEmgData(const DataTable<double>& emgData) {

        if (getNoMuscles() != emgData.getNColumns()) {
            std::cout << "Warning in TrialData::setEmgData: wrong number of columns and/or muscles \n";
        }

        emgData_ = emgData;
        hasEmgData_ = true;
    }

    void TrialData::setLmtMaData(const DataTable<double>& lmtData, const std::vector<DataTable<double>>& maData) {

        if (getNoMuscles() != lmtData.getNColumns())
            std::cout << "Warning in TrialData::setLmtMaData: wrong number of columns and/or muscles\n";
    
        if (getNoDofs() != maData.size())
            std::cout << "Warning in TrialData::setLmtMaData: wrong number of columns and/or dofs\n";
        
        lmtData_ = lmtData;
        maData_ = maData;
        hasLmtMaData_ = true;

    }

    void TrialData::setTorqueData(const DataTable<double>& torqueData) {

        if (getNoDofs() != torqueData.getNColumns())
            std::cout << "Warning in TrialData::setLmtMaData: wrong number of columns and/or dofs\n";

        torqueData_ = torqueData;
        hasTorqueData_;
    }

    void TrialData::setOpenSimData(
        const string& openSimModelFile,
        const string& externalLoadsFile,
        const string& motionFile) {

        openSimModelFile_ = openSimModelFile;
        externalLoadsFile_ = externalLoadsFile;
        updateKinematicData(motionFile);
        hasOpenSimData_ = true;
    }

    void TrialData::updateKinematicData(const string& motionFile, double fc) {

        OpenSim::Model model(openSimModelFile_);
        model.initSystem();
        OpenSim::Storage kinematics(motionFile);
        if (fc > 0) {
            kinematics.pad(kinematics.getSize() / 2.);
            kinematics.lowpassIIR(fc);
        }
        if (kinematics.isInDegrees()){
            model.getSimbodyEngine().convertDegreesToRadians(kinematics);
        }
        // Create differentiable splines of the coordinate data
        OpenSim::GCVSplineSet kinematicsSplines(5, &kinematics);

        //Functions must correspond to model coordinates and their order for the solver
        OpenSim::Array<string> coordinateNamesOS;
        model.getCoordinateSet().getNames(coordinateNamesOS);
        for (size_t i(0); i < coordinateNamesOS.getSize(); ++i){
            if (kinematicsSplines.contains(coordinateNamesOS.get(i))){
                kinematicsSplines.insert(i, kinematicsSplines.get(coordinateNamesOS.get(i)));
            }
            else{
                kinematicsSplines.insert(i, OpenSim::Constant(model.getCoordinateSet().get(i).getDefaultValue()));
            }
        }
        if (kinematicsSplines.getSize() > coordinateNamesOS.getSize()){
            kinematicsSplines.setSize(coordinateNamesOS.getSize());
        }

        //extract info from splines and generate tables
        vector<string> coordinateNames;
        ArrayConverter::toStdVector(coordinateNamesOS, coordinateNames);
        q_.setLabels(coordinateNames);
        qd_.setLabels(coordinateNames);
        qdd_.setLabels(coordinateNames);

        vector<double> timeColumn(lmtData_.getTimeColumn());
        for (auto& t : timeColumn) {
            OpenSim::Array<double> currentQ, currentQd, currentQdd;
            kinematicsSplines.evaluate(currentQ, 0, t);
            q_.pushRow(t, ArrayConverter::toStdVector(currentQ));

            kinematicsSplines.evaluate(currentQd, 1, t);
            qd_.pushRow(t, ArrayConverter::toStdVector(currentQd));

            kinematicsSplines.evaluate(currentQdd, 2, t);
            qdd_.pushRow(t, ArrayConverter::toStdVector(currentQdd));

        }
    }
    void TrialData::crop(double startTime, double stopTime) {

        if (hasEmgData_) {
            emgData_.crop(startTime, stopTime);
        }

        if (hasLmtMaData_) {
            lmtData_.crop(startTime, stopTime);
            for (auto& ma : maData_)
                ma.crop(startTime, stopTime);
        }
        
        if (hasTorqueData_) {
            torqueData_.crop(startTime, stopTime);
        }

        if (hasOpenSimData_) {
            q_.crop(startTime, stopTime);
            qd_.crop(startTime, stopTime);
            qdd_.crop(startTime, stopTime);
        }

    }
}