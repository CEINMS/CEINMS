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

namespace OpenSim {

    class ExternalForceFromQueue : public OpenSim::ExternalForce{
    public:
        using ExternalForceFrame = ceinms::QueueData < ceinms::ExternalForceData >;
        using ExternalForceQueue = ceinms::Concurrency::Queue < ExternalForceFrame >;
        ExternalForceFromQueue() = delete;
        ExternalForceFromQueue(std::shared_ptr<ExternalForceQueue> externalForceQueue,
            const std::string& appliedToBodyName,
            const std::string& forceExpressedInBodyName,
            const std::string& pointExpressedInBodyName);
        ExternalForceFromQueue(std::shared_ptr<ExternalForceQueue> externalForceQueue, SimTK::Xml::Element& aNode);
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
        std::shared_ptr<ExternalForceQueue> externalForceQueue_;
        mutable std::list<ceinms::QueueData<ceinms::ExternalForceData> > pastExternalForceFrames_;
        mutable double lastTime_;
        SimTK::ReferencePtr<const Body> appliedToBody_;
        SimTK::ReferencePtr<const Body> forceExpressedInBody_;
        SimTK::ReferencePtr<const Body> pointExpressedInBody_;
        mutable bool runCondition_;
    };
}



namespace ceinms {


    class PutInTheRightPlace {
    private:
        size_t inSize_, outSize_;
        std::vector<int> inToOutMapping_; //using int because in case in > out, we can use -1 as not-map token
        void updateMapping();
    public:
        PutInTheRightPlace() :
            inSize_(0), outSize_(0) {}
        PutInTheRightPlace(const std::vector<std::string>& inNames, const std::vector<std::string>& outNames) {
            setNames(inNames, outNames);
        }

        void setNames(const std::vector<std::string>& inNames, const std::vector<std::string>& outNames) {

            inSize_ = inNames.size();
            outSize_ = outNames.size();

            for (auto& in : inNames) {
                auto it(std::find(outNames.cbegin(), outNames.cend(), in));
                if (it != outNames.end())
                    inToOutMapping_.push_back(std::distance(outNames.cbegin(), it));
                else inToOutMapping_.push_back(-1);
            }
        }


        template<typename T>
        T&& map(const T& in) {
            T out(outSize_);
            for (auto i : inToOutMapping_) {
                if (i >= 0)
                    out[inToOutMapping_[i]] = in[i];
            }
            return std::move(out);
        }
    };

    class QueuesToXForces {
    public:
        using ExternalForcesFrame = QueueData < std::vector<ExternalForceData> >;
        using ExternalForcesQueue = Concurrency::Queue < ExternalForcesFrame >;

        template<typename NMSmodelT>
        QueuesToXForces(ExternalForcesQueue& externalForcesQueue, Concurrency::Latch& latch, const NMSmodelT& subject, const std::string& osimModelFilename, const std::string& externalLoadsXmlFilename, const std::vector< std::string >& jointsToLog);
        virtual ~QueuesToXForces();
        virtual void operator()() {};
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
        void initialiseContactModel();
        template<typename T>
        static typename T::type syncFrameToTime(double t, T& queue) {

            typename T::type frame;
            do {
                frame = queue.pop();
            } while (frame.time < t);
            return frame;
        };

        
        template<typename T1, typename T2>
        static std::tuple<typename T1::type, typename T2::type> syncFrames(T1& queue1, T2& queue2){


            auto frame1(queue1.pop());
            auto frame2(syncFrameToTime(frame1.time, queue2));
            return std::make_tuple(frame1, frame2);
        };

        template<typename T1, typename T2, typename T3>
        static std::tuple<typename T1::type, typename T2::type, typename T3::type> syncFrames(T1& queue1, T2& queue2, T3& queue3){


            auto frame1(queue1.pop());
            auto frame2(syncFrameToTime(frame1.time, queue2));
            auto frame3(syncFrameToTime(frame1.time, queue3));

            return std::make_tuple(frame1, frame2, frame3);
        };
        
    /*    template<typename A1, typename... Args>
        static std::tuple<typename A1::type, typename... Args::type> syncFrames(A1& queue1, Args&... queues){

            auto frame1(queue1.pop());
            return std::make_tuple(frame1, syncFrameToTime(frame1.time, queues...)...);
        };
*/
    private:

        using SingleExternalForceFrame = QueueData < ExternalForceData>;
        using SingleExternalForceQueue = Concurrency::Queue < SingleExternalForceFrame >;
        static SimTK::Vector&& toSimTKVector(const std::vector<double>& v);
        void parseExternalLoadsXml(const std::string& externalLoadsXmlFilename);
       
        void distributeExternalForces();

        void updateCeinmsToOpenSimDofMapping();
        void updateCeinmsToOpenSimMuscleMapping();
        void checkJointsToLogsExistInOpenSim();

        ExternalForcesQueue& inputExternalForcesQueue_;
        Concurrency::Latch splitForcesLatch_;
        Concurrency::Latch& externalLatch_;
        std::future<void> splitForcesThread_;
        std::vector<OpenSim::Force*> extForces_;
        std::vector<std::shared_ptr<SingleExternalForceQueue>> singleExternalForceQueues_;
        std::vector<std::string> jointsToLog_, dofNamesFromSubject_, musclesNamesFromSubject_;
        std::unique_ptr<JointContactFromOpensim> jointContactModel_;
        PutInTheRightPlace dofsMapping_, musclesMapping_;
        std::string osimModelFilename_, externalLoadsXmlFilename_;

    };



    template<typename NMSmodelT>
    QueuesToXForces::QueuesToXForces(
        ExternalForcesQueue& inputExternalForcesQueue,
        Concurrency::Latch& latch,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,
        const std::vector< std::string >& jointsToLog) :
        inputExternalForcesQueue_(inputExternalForcesQueue),
        externalLatch_(latch),
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