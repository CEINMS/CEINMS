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
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/QueueData.h"
#include "ceinms/ExternalForceData.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"
#include "ceinms/queues/SingleExternalForceQueue.h"
#include "ceinms/Mapper.h"


namespace OpenSim {

    class ExternalForceFromQueue : public OpenSim::ExternalForce{
    public:
        ExternalForceFromQueue() = delete;
        ExternalForceFromQueue(
            std::shared_ptr<ceinms::SingleExternalForceQueue> externalForceQueue,
            const std::string& appliedToBodyName,
            const std::string& forceExpressedInBodyName,
            const std::string& pointExpressedInBodyName);
        ExternalForceFromQueue(
            std::shared_ptr<ceinms::SingleExternalForceQueue> externalForceQueue, SimTK::Xml::Element& aNode);
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
        std::shared_ptr<ceinms::SingleExternalForceQueue> externalForceQueue_;
        mutable std::list<ceinms::QueueData<ceinms::ExternalForceData> > pastExternalForceFrames_;
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
            Concurrency::Latch& doneWithSubscriptions, 
            Concurrency::Latch& doneWithExecution,
            const NMSmodelT& subject, 
            const std::string& osimModelFilename, 
            const std::string& externalLoadsXmlFilename, 
            const std::vector< std::string >& jointsToLog);
        virtual ~QueuesToXForces();
        virtual void operator()() = 0;
        void setState(double time, const std::vector<double>& q, const std::vector<double>& qd, const std::vector<double>& qdd);
        void calculateIntersegmentalForces();
        //Instead of calculating the ID internally, I can provide the mobility forces (e.g. I've done the calculation before.. and I want to save time)
        void calculateIntersegmentalForces(const std::vector<double>& realMobilityForces); 
        void calculateJointReactionForces(const std::vector<double>& muscleForces, const std::vector<double>& estimatedMobilityForces);
        void calculateJointReactionForces(const std::vector<double>& muscleForces, const std::vector<double>& estimatedMobilityForces, const std::vector<double>& realMobilityForces);
        void getJointReactionForcesInG(std::vector<SimTK::Vec3>& jointReactionsInG) const;
        void getIntersegmentalForcesInG(std::vector<SimTK::Vec3>& intersegmentalForcesInG) const;
        void getJointReactionForcesInChild(std::vector<SimTK::Vec3>& jointReactionsInChild) const;
        void getIntersegmentalForcesInChild(std::vector<SimTK::Vec3>& intersegmentalForcesInChild) const;
        void doneWithSubscriptions();
        void doneWithExecution();
        void initialiseContactModel();

    private:
        static SimTK::Vector toSimTKVector(std::vector<double> v);
        void parseExternalLoadsXml(const std::string& externalLoadsXmlFilename);
        void distributeExternalForces();
        void updateCeinmsToOpenSimDofMapping();
        void updateCeinmsToOpenSimMuscleMapping();
        void checkJointsToLogsExistInOpenSim();

        MultipleExternalForcesQueue& inputExternalForcesQueue_;
        Concurrency::Latch splitForcesLatch_;
        Concurrency::Latch& externalDoneWithSubscriptionsLatch_;
        Concurrency::Latch& externalDoneWithExecutionLatch_;
        std::future<void> splitForcesThread_;
        std::vector<OpenSim::Force*> extForces_;
        std::vector<std::shared_ptr<SingleExternalForceQueue>> singleExternalForceQueues_;
        std::vector<std::string> jointsToLog_, dofNamesFromSubject_, musclesNamesFromSubject_;
        std::unique_ptr<JointContactFromOpensim> jointContactModel_;
        Mapper  dofsMapping_, musclesMapping_;
        std::string osimModelFilename_, externalLoadsXmlFilename_;

    };



    template<typename NMSmodelT>
    QueuesToXForces::QueuesToXForces(
        MultipleExternalForcesQueue& inputExternalForcesQueue,
        Concurrency::Latch& doneWithSubscriptions,
        Concurrency::Latch& doneWithExecution,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,
        const std::vector< std::string >& jointsToLog) :
        inputExternalForcesQueue_(inputExternalForcesQueue),
        externalDoneWithSubscriptionsLatch_(doneWithSubscriptions),
        externalDoneWithExecutionLatch_(doneWithExecution),
        jointContactModel_(nullptr),
        splitForcesLatch_(2), osimModelFilename_(osimModelFilename),
        externalLoadsXmlFilename_(externalLoadsXmlFilename),
        jointsToLog_(jointsToLog)

    {
        subject.getDoFNames(dofNamesFromSubject_);
        subject.getMuscleNames(musclesNamesFromSubject_);

    }



}

#endif