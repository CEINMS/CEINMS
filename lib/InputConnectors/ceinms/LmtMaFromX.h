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

#ifndef ceinms_LmtMaFromX_h
#define ceinms_LmtMaFromX_h

#include <vector>
#include <string>
#include "InputConnectors.h"

namespace ceinms {
    class LmtMaFromX
    {
    public:
        template <typename NMSModelT>
        LmtMaFromX(InputConnectors& inputConnectors, const NMSModelT& subject);
        virtual ~LmtMaFromX();
        virtual void operator()() {};
        void updateLmt(const std::vector<double>& currentLmtData, double currentTime);
        void updateMomentArms(const std::vector<double>& currentMomentArmsData, double currentTime, unsigned int whichDof);
        void updateMomentArmDerivatives(const std::vector<double>& currentMomentArmDerivativesData, double currentTime, unsigned int whichDof);
    protected:
        std::vector< std::string > musclesNames_;
        std::vector< std::string > dofNames_;
        std::vector< std::vector< std::string > > muscleNamesOnDofs_;
        InputConnectors& inputConnectors_;
    };


    template <typename NMSModelT>
    LmtMaFromX::LmtMaFromX(InputConnectors& inputConnectors, const NMSModelT& subject) : inputConnectors_(inputConnectors)
    {
        subject.getMuscleNames(musclesNames_);
        subject.getDoFNames(dofNames_);
        subject.getMuscleNamesOnDofs(muscleNamesOnDofs_);
        /*
        std::cout << "SIZE:" << CEINMS::InputConnectors::queueMomentArms.size() << std::endl;
        CEINMS::InputConnectors::queueMomentArms.resize(3);
        */
        //   for (int i= 0; i < dofNames_.size(); ++i) {
        //     CEINMS::Concurrency::Queue< std::vector<double> > newQueue;
        //     CEINMS::InputConnectors::queueMomentArms.push_back(newQueue);
        //   }
    }
}
#endif
