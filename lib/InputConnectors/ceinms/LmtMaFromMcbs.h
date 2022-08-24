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
* Author(s): E. Ceseracciu, C. Pizzolato, M. Reggiani                        *
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

#ifndef ceinms_LmtMaFromMcbs_h
#define ceinms_LmtMaFromMcbs_h

#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include <OpenSim/OpenSim.h>

#include "ceinms/LmtMaFromX.h"
#include "ceinms/queues/GeneralisedCoordinatesQueue.h"
#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"
#include "ceinms/Mapper.h"

#include "rtb/concurrency/Concurrency.h"

#include <mcbs/Spline.h>
#include <mcbs/SplineSetCreator.h>

//#include <osimSpliner/SplineSet.h>

namespace ceinms {
    
    class LmtMaFromMcbs : public LmtMaFromX {
    public:
		//coeffDir contains dofAngles.cfg
        template<typename NMSModelT>
        LmtMaFromMcbs(
            GeneralisedCoordinatesQueue& inputCoordinateQueue,
            LmtQueue& outputLmtQueue,
            MultipleMomentArmsQueues& outputMomentArmsQueue,
			rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
            const NMSModelT& subject,
            const std::string& coeffDir);

        template<typename NMSModelT>
        LmtMaFromMcbs(
            GeneralisedCoordinatesQueue& inputCoordinateQueue,
            LmtQueue& outputLmtQueue,
            MultipleMomentArmsQueues& outputMomentArmsQueue,
			rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
            const NMSModelT& subject,
            const std::string& coeffDir,
            const std::string& osimModelFilename);

        void operator()() override;

    private:
        void readDefaultMomentArmsFromOsim(
            const std::string& osimModelFIlename);
        std::shared_ptr<mcbs::SplineSetBase> splineSet_;
        GeneralisedCoordinatesQueue& inputCoordinateQueue_;
        std::map<std::string, std::vector<std::string>> muscleNamesOnDofsMap_;
        std::map<std::string, std::vector<double>> defaultMomentArmsOnDofsMap_;
        std::vector<std::string> dofNamesFromSpline_, dofsNamesFromOsimModel_;
        Mapper dofMapper_;
    };

    template<typename NMSModelT>
    LmtMaFromMcbs::LmtMaFromMcbs(
        GeneralisedCoordinatesQueue& inputCoordinateQueue,
		LmtQueue& outputLmtQueue,
        MultipleMomentArmsQueues& outputMomentArmsQueue,
		rtb::Concurrency::Latch& doneWithSubscriptions,
		rtb::Concurrency::Latch& doneWithExecution,
        const NMSModelT& subject,
        const std::string& coeffDir) :
        LmtMaFromX(outputLmtQueue, outputMomentArmsQueue, doneWithSubscriptions, doneWithExecution, subject),
        splineSet_(mcbs::SplineSetCreator::makeSplineSetmakeSplineSet(coeffDir, musclesNames_)),
        splineSet_(coeffDir, musclesNames_),
        inputCoordinateQueue_(inputCoordinateQueue) {

        subject.getMuscleNamesOnDofs(muscleNamesOnDofsMap_);

        if (N != dofNames_.size()) {
            std::cout << "ERROR: The number of dofs and the order of the MCBS are different";
            exit(EXIT_FAILURE);
        }

        dofNamesFromSpline_ = splineSet_.getDofNames();
        std::cout << "dof names from spline ";
        for (auto dof : dofNamesFromSpline_)
            std::cout << dof << " ";
        std::cout << std::endl;

        dofMapper_.setNames(dofNames_, dofNamesFromSpline_);

        // inputCoordinateQueue_should receive data already formatted correctly because
        // this object doesn't know opensim
    }

    template<typename NMSModelT>
    LmtMaFromMcbs::LmtMaFromMcbs(
        GeneralisedCoordinatesQueue& inputCoordinateQueue,
		LmtQueue& outputLmtQueue,
        MultipleMomentArmsQueues& outputMomentArmsQueue,
		rtb::Concurrency::Latch& doneWithSubscriptions,
		rtb::Concurrency::Latch& doneWithExecution,
        const NMSModelT& subject,
        const std::string& coeffDir,
        const std::string& osimModelFilename) :
        LmtMaFromX(outputLmtQueue, outputMomentArmsQueue, doneWithSubscriptions, doneWithExecution, subject),
        splineSet_(mcbs::SplineSetCreator::makeSplineSet(coeffDir, musclesNames_)),
        inputCoordinateQueue_(inputCoordinateQueue) {

        subject.getMuscleNamesOnDofs(muscleNamesOnDofsMap_);
        dofNamesFromSpline_ = splineSet_->getDofNames();
        for (auto& dof : dofNames_) {
            auto it(std::find(dofNamesFromSpline_.begin(), dofNamesFromSpline_.end(), dof));
            if (it == dofNamesFromSpline_.end())
                dofsNamesFromOsimModel_.emplace_back(dof);
        }

        for (auto& dof : dofsNamesFromOsimModel_)
            std::cout << dof << " not included in MCBS, reading from Osim model" << std::endl;

        readDefaultMomentArmsFromOsim(osimModelFilename);

        std::cout << "dof names from spline ";
        for (auto dof : dofsNamesFromOsimModel_)
            std::cout << dof << " ";
        std::cout << std::endl;

        dofMapper_.setNames(dofNames_, dofNamesFromSpline_);

        // inputCoordinateQueue_should receive data already formatted correctly because
        // this object doesn't know opensim
    }
}

#endif
