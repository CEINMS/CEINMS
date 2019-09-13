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

#ifndef ceinms_QueuesToXForces_h
#define ceinms_QueuesToXForces_h
#include <string>
#include <vector>
#include <memory>
#include <future>
#include <algorithm>
#include <OpenSim/OpenSim.h>
#include "ceinms/JointContactFromOpensim.h"
#include "rtb/concurrency/Concurrency.h"
#include "ceinms/QueueData.h"
#include "ceinms/ExternalForceData.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"
#include "ceinms/queues/SingleExternalForceQueue.h"
#include "ceinms/Mapper.h"
#include "ceinms/ExternalForceDataSetup.h"

namespace OpenSim {

    class ExternalForceFromQueue : public OpenSim::ExternalForce{
    public:
        ExternalForceFromQueue() = delete;
        ExternalForceFromQueue(ExternalForceFromQueue&) = delete;
        ExternalForceFromQueue(
            ceinms::SingleExternalForceQueue* externalForceQueue,
            const std::string& forceIdentifier,
            const std::string& pointIdentifier,
            const std::string& torqueIdentifier,
            const std::string& appliedToBodyName,
            const std::string& forceExpressedInBodyName,
            const std::string& pointExpressedInBodyName);
        ExternalForceFromQueue(
            ceinms::SingleExternalForceQueue* externalForceQueue, SimTK::Xml::Element& aNode);
        virtual ~ExternalForceFromQueue();
    protected:
        virtual void computeForce(const SimTK::State& state,
            SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
            SimTK::Vector& generalizedForces) const override;
        void connectToModel(Model& model) override;
    private:
        bool isSubscribed;
        ceinms::ExternalForceData getExternalForceDataFromPastValues(double time) const;
        ceinms::ExternalForceData getExternalForceDataFromQueue(double time) const;
        //need to use raw pointers because opensim deletes stuff for me.. and it's a mess
        ceinms::SingleExternalForceQueue* externalForceQueue_;
        mutable std::list<ceinms::QueueData<ceinms::ExternalForceData> > pastExternalForceFrames_;
        static const unsigned MaxStorageLength_ = 1000;
        mutable double lastTime_;
        SimTK::ReferencePtr<const Body> appliedToBody_;
        SimTK::ReferencePtr<const Body> forceExpressedInBody_;
        SimTK::ReferencePtr<const Body> pointExpressedInBody_;
        mutable bool runCondition_;
    };
}

namespace ceinms {

    class QueuesToXForces {
    public:

        template<typename NMSmodelT>
        QueuesToXForces(
            MultipleExternalForcesQueue& inputExternalForcesQueue,
			rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
            const NMSmodelT& subject,
            const std::string& osimModelFilename,
            const std::string& externalLoadsXmlFilename,
            const std::vector< std::string >& jointsToLog);

        template<typename NMSmodelT>
        QueuesToXForces(
            MultipleExternalForcesQueue& inputExternalForcesQueue,
			rtb::Concurrency::Latch& doneWithSubscriptions,
			rtb::Concurrency::Latch& doneWithExecution,
            const NMSmodelT& subject,
            const std::string& osimModelFilename,
            const std::vector<ExternalForceDataSetup>& externalForceDataSetups,
            const std::vector< std::string >& jointsToLog);

        virtual ~QueuesToXForces();
        virtual void operator()() = 0;
        void setState(double time, const std::vector<double>& q, const std::vector<double>& qd, const std::vector<double>& qdd);
        void calculateIntersegmentalForces();
        //Instead of calculating the ID internally, I can provide the mobility forces (e.g. I've done the calculation before.. and I want to save time)
        void calculateIntersegmentalForces(const std::vector<double>& realMobilityForces);
        void calculateJointReactionForces(const std::vector<double>& muscleForces, const std::vector<double>& estimatedMobilityForces);
        void calculateJointReactionForces(const std::vector<double>& muscleForces, const std::vector<double>& estimatedMobilityForces, const std::vector<double>& realMobilityForces);
        void calculateInverseDynamics();
        void getJointReactionForcesInG(std::vector<SimTK::Vec3>& jointReactionsInG) const;
        void getIntersegmentalForcesInG(std::vector<SimTK::Vec3>& intersegmentalForcesInG) const;
        void getJointReactionForcesInChild(std::vector<SimTK::Vec3>& jointReactionsInChild) const;
        void getIntersegmentalForcesInChild(std::vector<SimTK::Vec3>& intersegmentalForcesInChild) const;
        void getResidualMobilityForces(std::vector<double>& residualMobilityForces, bool limitToSubjectDofs) const; //i.e. result of ID
        void doneWithSubscriptions();
        void doneWithExecution();
        void initialiseContactModel();

    private:
        static SimTK::Vector toSimTKVector(std::vector<double> v);
        static std::vector<double> fromSimTKVector(SimTK::Vector v);
        void distributeExternalForces();
        void updateCeinmsToOpenSimDofMapping();
        void updateCeinmsToOpenSimMuscleMapping();
        void checkJointsToLogsExistInOpenSim();
        void initialiseExternalForces();
        MultipleExternalForcesQueue& inputExternalForcesQueue_;
		rtb::Concurrency::Latch splitForcesLatch_;
		rtb::Concurrency::Latch& externalDoneWithSubscriptionsLatch_;
		rtb::Concurrency::Latch& externalDoneWithExecutionLatch_;
        std::future<void> splitForcesThread_;
        std::vector<OpenSim::Force*> extForces_;
        std::vector<SingleExternalForceQueue*> singleExternalForceQueues_;
        std::vector<std::string> jointsToLog_, dofNamesFromSubject_, musclesNamesFromSubject_;
        std::unique_ptr<JointContactFromOpensim> jointContactModel_;
        Mapper  dofsMapping_, musclesMapping_;
        std::string osimModelFilename_;
        std::vector<ExternalForceDataSetup> externalForcesDataSetups_;
    };

    template<typename NMSmodelT>
    QueuesToXForces::QueuesToXForces(
        MultipleExternalForcesQueue& inputExternalForcesQueue,
		rtb::Concurrency::Latch& doneWithSubscriptions,
		rtb::Concurrency::Latch& doneWithExecution,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,
        const std::vector< std::string >& jointsToLog) :
        inputExternalForcesQueue_(inputExternalForcesQueue),
        externalDoneWithSubscriptionsLatch_(doneWithSubscriptions),
        externalDoneWithExecutionLatch_(doneWithExecution),
        jointContactModel_(nullptr),
        splitForcesLatch_(2), osimModelFilename_(osimModelFilename),
        jointsToLog_(jointsToLog)

    {
        subject.getDoFNames(dofNamesFromSubject_);
        subject.getMuscleNames(musclesNamesFromSubject_);
        externalForcesDataSetups_ = parseExternalLoadsXml(externalLoadsXmlFilename);
    }

    template<typename NMSmodelT>
    QueuesToXForces::QueuesToXForces(
        MultipleExternalForcesQueue& inputExternalForcesQueue,
		rtb::Concurrency::Latch& doneWithSubscriptions,
		rtb::Concurrency::Latch& doneWithExecution,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::vector<ExternalForceDataSetup>& externalForceDataSetups,
        const std::vector< std::string >& jointsToLog) :
        inputExternalForcesQueue_(inputExternalForcesQueue),
        externalDoneWithSubscriptionsLatch_(doneWithSubscriptions),
        externalDoneWithExecutionLatch_(doneWithExecution),
        jointContactModel_(nullptr),
        splitForcesLatch_(2), osimModelFilename_(osimModelFilename),
        externalForcesDataSetups_(externalForceDataSetups),
        jointsToLog_(jointsToLog)

    {
        subject.getDoFNames(dofNamesFromSubject_);
        subject.getMuscleNames(musclesNamesFromSubject_);
    }
}

#endif
