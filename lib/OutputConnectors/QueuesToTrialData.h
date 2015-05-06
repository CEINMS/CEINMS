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
 * Author(s): C. Pizzolato, M. Reggiani, E. Ceseracciu                        *
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

#ifndef ceinms_QueuesToTrialData_h
#define ceinms_QueuesToTrialData_h

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "OutputConnectors.h"
#include "InputConnectors.h"
#include "TrialData.h"

namespace ceinms {
    class QueuesToTrialData {
    public:
        QueuesToTrialData() = delete;
        QueuesToTrialData(const QueuesToTrialData&) = delete;
        QueuesToTrialData& operator=(const QueuesToTrialData&) = delete;

        template<typename NMSmodel> QueuesToTrialData(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, NMSmodel& subject, std::string id) : inputConnectors_(inputConnectors), outputConnectors_(outputConnectors)
        {
            data_.id = id;
            std::vector<std::string> muscleNames;
            subject.getMuscleNames(muscleNames);
            data_.emgData.setLabels(muscleNames);
            data_.lmtData.setLabels(muscleNames);

            subject.getDoFNames(data_.dofNames);
            data_.noDoF = data_.dofNames.size();
            std::vector<std::vector<std::string>> maMuscleNames;
            subject.getMuscleNamesOnDofs(maMuscleNames);
            data_.maData.resize(data_.noDoF);
            for (auto i(0); i < data_.noDoF; ++i)
                data_.maData.at(i).setLabels(maMuscleNames.at(i));
            data_.torqueData.setLabels(data_.dofNames);

        };
        ~QueuesToTrialData() = default;
        void operator()();
        TrialData getTrialData();

    private:
        TrialData data_;
        InputConnectors& inputConnectors_;
        OutputConnectors& outputConnectors_;
        //std::vector<std::string> valuesToWrite_;
        //std::vector<std::string> torqueNames_;
        //std::vector<std::string> muscleNames_;
    };
}
#endif
