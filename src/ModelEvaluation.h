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

#ifndef ModelEvaluation_h
#define ModelEvaluation_h

#include <vector>
#include <string>
#include "NMSmodel.h"
#include "SetupDataStructure.h"

#include "ExponentialActivation.h"
#include "StiffTendon.h"
#include "ElasticTendon.h"
#include "ElasticTendon_BiSec.h"

typedef StiffTendon MyTendonType;


class ModelEvaluation
{
  public:
    typedef NMSmodel<ExponentialActivation, MyTendonType> MyNMSmodel;
    typedef SetupDataStructure<ExponentialActivation, MyTendonType> MySetupDataStructure;
    ModelEvaluation(const std::string& configurationFile);
    ~ModelEvaluation();
    void operator()();
  private:
    void popEmgFront(std::vector<double>& emgFromQueue);
    void popLmtFront(std::vector<double>& lmtFromQueue);
    void popMomentArmsFront(std::vector<double>& momentArmsFromQueue, unsigned int whichDof);
    void popExternalTorqueFront(std::vector<double>& externalTorqueFromQueue, unsigned int whichDof);
    void getMusclesNames();
    void setDofNames();
    void getDofNamesAssociatedToExternalTorque();
    void initOfflineCurve(MyNMSmodel& mySubject);
    std::string configurationFile_;
    std::vector< std::string > musclesNames_;
    std::vector< std::vector < std::string > > musclesNamesOnDof_;
    std::vector< std::string > dofNames_;
    std::vector< std::string > dofNamesWithExtTorque_;
    unsigned int noDof_;
};


#endif
