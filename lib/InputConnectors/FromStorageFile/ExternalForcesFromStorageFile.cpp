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
* Author(s): E. Ceseracciu, M. Reggiani                                      *
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

#include "ceinms/ExternalForcesFromStorageFile.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/FileUtils.h"
#include <boost/filesystem.hpp>
using boost::filesystem::path;
#include <OpenSim/OpenSim.h>
using OpenSim::Model;
#include <string>
using std::string;
#include <vector>
using std::vector;

namespace ceinms {

    ExternalForcesFromStorageFile::ExternalForcesFromStorageFile(
        MultipleExternalForcesQueue& outputMultipleExternalForcesQueue,
        Concurrency::Latch& doneWithSubscriptions,
        Concurrency::Latch& doneWithExecution,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename) :
        ExternalForcesFromX(outputMultipleExternalForcesQueue, doneWithSubscriptions, doneWithExecution),
        osimModelFilename_(osimModelFilename),
        externalLoadsXmlFilename_(externalLoadsXmlFilename)
    {}

    void ExternalForcesFromStorageFile::operator()() {

        Model model(osimModelFilename_);
        OpenSim::ExternalLoads externalLoads(model, externalLoadsXmlFilename_);

        path p(externalLoadsXmlFilename_);
        path baseDir(p.parent_path());

        //correct file path so it works relatively to the xml itself
        auto forcesFilename = externalLoads.getDataFileName();
        if (!FileUtils::isAbsolute(forcesFilename.c_str()))
            forcesFilename = (baseDir / forcesFilename).string();
        externalLoads.setDataFileName(forcesFilename);

        auto kinematicsFilename = externalLoads.getExternalLoadsModelKinematicsFileName();
        if (!FileUtils::isAbsolute(kinematicsFilename.c_str()))
            kinematicsFilename = (baseDir / kinematicsFilename).string();
        externalLoads.setExternalLoadsModelKinematicsFileName(kinematicsFilename);

        externalLoads.invokeConnectToModel(model);

        OpenSim::Storage externalForcesStorage(externalLoads.getDataFileName());
        OpenSim::Array<double> timeColumnExternalLoadsOS;
        vector<double> timeColumnExternalLoads;
        externalForcesStorage.getTimeColumn(timeColumnExternalLoadsOS);
        ArrayConverter::toStdVector(timeColumnExternalLoadsOS, timeColumnExternalLoads);

        ExternalForcesFromX::doneWithSubscriptions();
        for (auto t : timeColumnExternalLoads) {
            MultipleExternalForcesFrame externalForcesFrame;
            externalForcesFrame.time = t;
            for (unsigned i(0); i < externalLoads.getSize(); ++i) {
                ExternalForceData currentExtForce;
                currentExtForce.setForceVector(externalLoads.get(i).getForceAtTime(t));
                currentExtForce.setApplicationPoint(externalLoads.get(i).getPointAtTime(t));
                currentExtForce.setTorque(externalLoads.get(i).getTorqueAtTime(t));
                externalForcesFrame.data.push_back(currentExtForce);
            }
            ExternalForcesFromX::updateExternalForces(externalForcesFrame.data, externalForcesFrame.time);
        }
        ExternalForcesFromX::sendEndOfData();
        ExternalForcesFromX::doneWithExecution();
    }
}