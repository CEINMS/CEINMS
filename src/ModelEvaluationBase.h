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

#ifndef ceinms_ModelEvaluationBase_h
#define ceinms_ModelEvaluationBase_h

#include <vector>
#include <string>

#include "InputConnectors.h"
#include "OutputConnectors.h"

namespace ceinms {
    template <typename Logger>
    class ModelEvaluationBase {

    public:
        ModelEvaluationBase() = delete;
        ModelEvaluationBase(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, const std::vector<std::string>& valuesToLog);
        virtual ~ModelEvaluationBase();
        virtual void operator()() = 0;

    protected:
        void getEmgFromInputQueue(InputConnectors::FrameType& emgFromQueue);
        void getLmtFromInputQueue(InputConnectors::FrameType& lmtFromQueue);
        void getMomentArmsFromInputQueue(InputConnectors::FrameType& momentArmsFromQueue, unsigned int whichDof);
        void getExternalTorquesFromInputQueue(InputConnectors::FrameType& externalTorquesFromQueue);
        void doneWithExecution();

        InputConnectors::FrameType getEmgFromInputQueue();
        InputConnectors::FrameType getLmtFromInputQueue();
        InputConnectors::FrameType getMomentArmsFromInputQueue(unsigned int whichDof);
        InputConnectors::FrameType getExternalTorquesFromInputQueue();

        void subscribeToInputConnectors();
        bool externalTorquesAvailable() const;
        float getGlobalTimeLimit() const;

        Logger logger;

    private:
        InputConnectors& inputConnectors_;
        OutputConnectors& outputConnectors_;
    };
}

#include "ModelEvaluationBase.cpp"
#endif
