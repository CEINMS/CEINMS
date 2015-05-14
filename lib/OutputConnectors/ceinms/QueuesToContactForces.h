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

#ifndef ceinms_QueuesToContactForces_h
#define ceinms_QueuesToContactForces_h

#include "ceinms/concurrency/Latch.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/QueueData.h"
#include "ceinms/ExternalForceData.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/OutputConnectors.h"
#include <memory>
#include <OpenSim/OpenSim.h>
#include <ostream>
#include <tuple>

namespace OpenSim {

    class ExternalForceFromQueue : public OpenSim::ExternalForce{
    public:
        using ExternalForceFrame = ceinms::QueueData < ceinms::ExternalForceData > ;
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
   
    /*
    template<typename Arg>
    static auto syncFrameToTime(double t, Arg& arg)  -> decltype(typename Arg::type){

        do {
            auto frame(args.pop());
        } while (frame.time < t);
        return frame;
    };

    template<typename... Args>
    static auto syncFrames(Args&... args) -> decltype(std::tuple<typename Args::type...>) {
        

        auto frames(std::make_tuple(args.pop()...));
        (std::get<sizeof... Args>(frames)).time;


    };
    */




    class QueuesToContactForces {
    public:
        using MuscleForcesFrame = QueueData < std::vector<double> >;
        using MuscleForcesQueue = Concurrency::Queue < MuscleForcesFrame >;

        using ContactForcesFrame = QueueData < std::vector<SimTK::Vec3> >;
        using ContactForcesQueue = Concurrency::Queue < ContactForcesFrame > ;

        using ExternalForcesFrame = QueueData < std::vector<ExternalForceData> >;
        using ExternalForcesQueue = Concurrency::Queue < ExternalForcesFrame > ;

        using GeneralisedCoordinatesFrame = QueueData < GeneralisedCoordinatesData >;
        using GeneralisedCoordinatesQueue = Concurrency::Queue < GeneralisedCoordinatesFrame > ;
        
        using EstimatedTorquesFrame = QueueData < OutputConnectors::FrameType > ;
        using EstimatedTorquesQueue = Concurrency::Queue < EstimatedTorquesFrame > ;

        QueuesToContactForces() = delete;
        QueuesToContactForces(const QueuesToContactForces&) = delete;
        QueuesToContactForces& operator=(const QueuesToContactForces&) = delete;

        //this will calculate intersegmental forces
        template <typename NMSmodelT>
        QueuesToContactForces(
            GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue,
            ExternalForcesQueue& inputExternalForcesQueue,
            ContactForcesQueue& outputIntersegmentalForcesQueue,
            Concurrency::Latch& latch,
            const NMSmodelT& subject,
            const std::string& osimModelFilename,
            const std::string& externalLoadsXmlFilename,    //this is the same as openSim
            const std::vector< std::string >& jointsToLog);


        //this will calculate joint reactions
        template <typename NMSmodelT>
        QueuesToContactForces(
            GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue,
            ExternalForcesQueue& inputExternalForcesQueue,
            MuscleForcesQueue& inputMuscleForcesQueue,
            EstimatedTorquesQueue& inputEstimatedTorquesQueue,
            ContactForcesQueue& outputJointReactionForcesQueue,
            Concurrency::Latch& latch,
            const NMSmodelT& subject,
            const std::string& osimModelFilename,
            const std::string& externalLoadsXmlFilename,    //this is the same as openSim
            const std::vector< std::string >& jointsToLog);


        //this will calculate joint reactions and intersegmental forces
        template <typename NMSmodelT>
        QueuesToContactForces(
            GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue,
            ExternalForcesQueue& inputExternalForcesQueue,
            MuscleForcesQueue& inputMuscleForcesQueue,
            EstimatedTorquesQueue& inputEstimatedTorquesQueue,
            ContactForcesQueue& outputJointReactionForcesQueue,
            ContactForcesQueue& outputIntersegmentalForcesQueue,
            Concurrency::Latch& latch,
            const NMSmodelT& subject,
            const std::string& osimModelFilename,
            const std::string& externalLoadsXmlFilename,    //this is the same as openSim
            const std::vector< std::string >& jointsToLog);



        ~QueuesToContactForces() = default;
        void operator()();

    private:

        template<typename T>
        static typename T::type syncFrameToTime(double t, T& queue) {

            do {
                auto frame(queue.pop());
            } while (frame.time < t);
            return frame;
        };


        template<typename T1, typename T2>
        static std::tuple<typename T1::type, typename T2::type> syncFrames(T1& queue1, T2& queue2){


            auto frame1(queue1.pop());
            auto frame2(syncFrameToTime(frame1.time, queue2));
            return std::make_tuple(frame1, frame2);
        };


        using SingleExternalForceFrame = QueueData < ExternalForceData>;
        using SingleExternalForceQueue = Concurrency::Queue < SingleExternalForceFrame >;

        GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue_;
        ExternalForcesQueue& inputExternalForcesQueue_;
        MuscleForcesQueue& inputMuscleForcesQueue_;
        EstimatedTorquesQueue& inputEstimatedTorquesQueue_;
        ContactForcesQueue& outputJointReactionForcesQueue_;
        ContactForcesQueue& outputIntersegmentalForcesQueue_;
        std::string modelFilename_;
        Concurrency::Latch& latch_;
        std::vector<std::string> jointsToLog_, dofNames_, musclesNames_;

        std::vector<std::shared_ptr<SingleExternalForceQueue>> singleExternalForceQueues_;
        std::vector<OpenSim::Force*> extForces_;
        void parseExternalLoadsXml(const std::string& externalLoadsXmlFilename);

        bool calculateJointReactions_, calculateIntersegmentalForces_, useEstimatedTorques_;
    };
}

#include "QueuesToContactForces.cpp"
#endif