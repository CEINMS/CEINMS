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

#ifndef Ceinms_CalibrationManager_h
#define Ceinms_CalibrationManager_h
#include <string>
#include <list>
#include "ceinms/CeinmsCalibrationSetupXmlReader.h"
#include "ceinms/CalibrationXmlReader.h"
#include "ceinms/InputDataXmlReader.h"
#include "ceinms/TrialData.h"


namespace ceinms {

    class CalibrationManager {

    public:
        CalibrationManager(const std::string& setupFilename);
        bool run();

    private:
        template <typename NMSmodelT>
        bool runOpenLoop();
        template <typename NMSmodelT>
        bool runHybrid();
        template <typename NMSmodelT, template <typename> class OptimizerT>
        bool runMinimiseTorque(const CalibrationStep& currentCalibrationStep);

        template <typename NMSmodelT>
        static TrialData readTrialData(const std::string& inputDataFilename,
                                       NMSmodelT& mySubject,
                                       const std::string& trialId,
                                       const std::string& emgGeneratorFile);
        static void sortMaFilenames(const std::map<std::string,
                                    std::string>& maMap,
                                    const std::vector< std::string >& dofNames,
                                    std::vector< std::string >& maDataFilenames);

        template<typename NMSmodelT>
        static void writeXml(NMSmodelT& mySubject,
                             const std::string& uncalibratedSubjectXmlFile,
                             const string& calibrationXmlFile,
                             const string& newCalibratedSubjectXmlFile);

        CeinmsCalibrationSetupXmlReader ceinmsCalibrationSetup_;
        CalibrationXmlReader calibrationXmlReader_;
        std::string uncalibratedSubjectFile_;
        std::string calibratedSubjectFile_;
        std::string calibrationFile_;
        std::string emgGeneratorFile_;
        std::list<std::string> calibrationTrialIDs_;

    };
}

#include "CalibrationManager.cpp"
#endif
