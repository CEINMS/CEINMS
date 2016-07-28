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
* Author(s): C. Pizzolato                                                    *
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

#include "ceinms/SumGenericContactForces.h"
#include <vector>
#include <cmath>
using std::vector;
#include <string>
using std::string;

namespace ceinms {
    
    SumGenericContactForces::SumGenericContactForces(const ObjectiveFunctionSetup& setup) : 
        f_(std::numeric_limits<double>::max())
    {
        setSetup(setup);

    }

    void SumGenericContactForces::setSetup(const ObjectiveFunctionSetup& setup) {

        weight_ = setup.weight;
        exponent_ = setup.exponent;
    }

    void SumGenericContactForces::setContactModelSetup(const ContactModelFromOpenSimSetup& setups) {

        
        barrierFunc_.initialize(setup.barrierListSetup, contactModel_.getCondyleNames());

    }

    void SumGenericContactForces::initialiseContactModels() {

        for (unsigned i(0); i < contactModelSetups_.size(); ++i) {
            string modelFilename(contactModelSetups_.at(i).osimModel);
            string extLoadsXml(contactModelSetups_.at(i).externalLoads);
            contactModels_.emplace_back(new JointContactFromOpensim(modelFilename, extLoadsXml));
        }

        std::vector<std::string> coordinateNamesFromOsim, dofNamesFromSubject, muscleNamesFromOsim, musclesNamesFromSubject, jointNamesFromOsim;
        jointContactModel_->getCoordinateNamesFromOsimModel(coordinateNamesFromOsim);
        dofsMapping_.setNames(dofNamesFromSubject_, coordinateNamesFromOsim);
        jointContactModel_->getMuscleNamesFromOsimModel(muscleNamesFromOsim);
        musclesMapping_.setNames(musclesNamesFromSubject_, muscleNamesFromOsim);

        jointContactModel_->getJointNamesFromOsimModel(jointNamesFromOsim);
        for (auto& jointToLog : jointsToLog_) {
            auto it(std::find(jointNamesFromOsim.cbegin(), jointNamesFromOsim.cend(), jointToLog));
            if (it == jointNamesFromOsim.end()) {
                std::cout << jointToLog << " does not exist in the OpenSim model\nExit.";
                exit(EXIT_FAILURE);
            }
        }
    }




    void SumGenericContactForces::calculate(const vector<Result>& results) {

        f_ = 0;
        vector<double> currentContactForces;
        auto tIt(trials_.cbegin());
        auto rIt(results.cbegin());
        size_t trialIndex(0);
        for (tIt; tIt != trials_.cend(); ++tIt, ++rIt, ++trialIndex) {

            size_t nRows(std::min(tIt->torqueData.getNRows(), rIt->torques.getNRows()));
            contactForces_.at(trialIndex).resize(nRows);
            for (size_t rowIdx(0); rowIdx < nRows; ++rowIdx) {
                kneeContactModel_.setTorques(tIt->torqueData.getRow(rowIdx), rIt->torques.getRow(rowIdx));
                kneeContactModel_.getContactForces(currentContactForces);
                contactForces_.at(trialIndex).at(rowIdx) = currentContactForces;
                for (auto& cf : currentContactForces)
                    f_ += weight_*std::pow(cf, exponent_);
                f_ += barrierFunc_.getValue(currentContactForces);
            }
        }
    }

    void SumGenericContactForces::printCosts() const {

        auto names = kneeContactModel_.getCondyleNames();
        for (size_t trialIndex(0); trialIndex < contactForces_.size(); ++trialIndex) {
            std::cout << "Trial #" << trialIndex + 1 << std::endl;

            vector<double> maxValues;
            auto val = std::max_element(
                contactForces_.at(trialIndex).begin(),
                contactForces_.at(trialIndex).end(),
                [](const std::vector<double>& lhs, const std::vector<double>& rhs){
                return lhs.front() < rhs.front();
            }

            );
            maxValues.push_back(val->front());

            val = std::max_element(
                contactForces_.at(trialIndex).begin(),
                contactForces_.at(trialIndex).end(),
                [](const std::vector<double>& lhs, const std::vector<double>& rhs){
                return lhs.back() < rhs.back();
            }

            );

            maxValues.push_back(val->back());
            std::cout << "max " << names.at(0) << " " << maxValues.at(1) << " N" << std::endl;
            std::cout << "max " << names.at(1) << " " << maxValues.at(0) << " N" << std::endl;

            std::cout << std::endl;
        }
    }
    
}