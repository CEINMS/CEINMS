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

#include "ceinms/QueuesToContactForces.h"
#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include <string>
#include <thread>
#include <limits>
#include <future>
#include <algorithm>

namespace OpenSim {

    ExternalForceFromQueue::ExternalForceFromQueue
        (std::shared_ptr<ExternalForceQueue> externalForceQueue,
        const std::string& appliedToBodyName,
        const std::string& forceExpressedInBodyName,
        const std::string& pointExpressedInBodyName) :
        externalForceQueue_(externalForceQueue),
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr),
        lastTime_(std::numeric_limits<double>::lowest()),
        isSubscribed(false),
        runCondition_(true)
    {
        setAppliedToBodyName(appliedToBodyName);
        setForceExpressedInBodyName(forceExpressedInBodyName);
        setPointExpressedInBodyName(pointExpressedInBodyName);
        
    }


    ExternalForceFromQueue::ExternalForceFromQueue(std::shared_ptr<ExternalForceQueue> externalForceQueue, SimTK::Xml::Element& node) :
        externalForceQueue_(externalForceQueue),
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr),
        lastTime_(std::numeric_limits<double>::lowest()),
        isSubscribed(false),
        runCondition_(true)
    {
        ExternalForce::updateFromXMLNode(node);
      
    }

    ceinms::ExternalForceData ExternalForceFromQueue::getExternalForceDataFromPastValues(double time) const {

        auto it(std::lower_bound(pastExternalForceFrames_.begin(), pastExternalForceFrames_.end(), time, [](
            ceinms::QueueData<ceinms::ExternalForceData>& lhs,
            double t){ return lhs.time < t; }));
        return it->data;

    }


    ceinms::ExternalForceData ExternalForceFromQueue::getExternalForceDataFromQueue(double time) const {
        
        ExternalForceFrame forceFrame;
        while (time > lastTime_ && runCondition_){
            forceFrame = externalForceQueue_->pop();
            if (forceFrame.time == std::numeric_limits<double>::infinity()) {
                runCondition_ = false;
                forceFrame.data.setForceVector(SimTK::Vec3(0.));
                forceFrame.data.setForceApplicationPoint(SimTK::Vec3(0.));
                forceFrame.data.setTorque(SimTK::Vec3(0.));
            }
            if (runCondition_) {
                pastExternalForceFrames_.push_back(forceFrame);
                lastTime_ = forceFrame.time;
            }
        }
        return forceFrame.data;
    }


    void ExternalForceFromQueue::computeForce(
        const SimTK::State& state,
        SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
        SimTK::Vector& generalizedForces) const
    {
        if (runCondition_) {
            double time(state.getTime());
            ceinms::ExternalForceData forceData;
            if (time <= lastTime_)
                forceData = getExternalForceDataFromPastValues(time);
            else
                forceData = getExternalForceDataFromQueue(time);

            //it could be possible to chenge the application point of the force on the fly, but at the moment we just keep the definitions from the xml
            const SimbodyEngine& engine = getModel().getSimbodyEngine();
            SimTK::Vec3 transformedForce, transformedPosition, transformedTorque;
            engine.transform(state, *forceExpressedInBody_, forceData.getForce(), engine.getGroundBody(), transformedForce);
            engine.transformPosition(state, *pointExpressedInBody_, forceData.getApplicationPoint(), *appliedToBody_, transformedPosition);
            ExternalForce::applyForceToPoint(state, *appliedToBody_, transformedPosition, transformedForce, bodyForces);
            engine.transform(state, *forceExpressedInBody_, forceData.getTorque(), engine.getGroundBody(), transformedTorque);
            ExternalForce::applyTorque(state, *appliedToBody_, transformedTorque, bodyForces);
        }
    }
 

    void ExternalForceFromQueue::connectToModel(Model& model) {

        if (!isSubscribed) {
            externalForceQueue_->subscribe();
            isSubscribed = true;
        }
        Super::connectToModel(model);
        const std::string& appliedToBodyName = get_applied_to_body();
        const std::string& forceExpressedInBodyName = get_force_expressed_in_body();

        appliedToBody_ = &_model->updBodySet().get(appliedToBodyName);
        forceExpressedInBody_ = &_model->updBodySet().get(forceExpressedInBodyName);
        pointExpressedInBody_ = &_model->updBodySet().get(get_point_expressed_in_body());
    }

    ExternalForceFromQueue::~ExternalForceFromQueue() {
        
   //     externalForceQueue_.unsubscribe();
    }
}


namespace ceinms {


    //this will calculate intersegmental forces
    template <typename NMSmodelT>
    QueuesToContactForces::QueuesToContactForces(
        GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue,
        ExternalForcesQueue& inputExternalForcesQueue,
        ContactForcesQueue& outputIntersegmentalForcesQueue,
        Concurrency::Latch& latch,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,    //this is the same as openSim
        const std::vector< std::string >& jointsToLog):
        inputGeneralisedCoordinatesQueue_(inputGeneralisedCoordinatesQueue),
        inputExternalForcesQueue_(inputExternalForcesQueue),
        outputIntersegmentalForcesQueue_(outputIntersegmentalForcesQueue),
        latch_(latch),
        modelFilename_(osimModelFilename),
        jointsToLog_(jointsToLog),
        calculateJointReactions_(false),
        calculateIntersegmentalForces_(true),
        useEstimatedTorques_(false){

        subject.getDoFNames(dofNames_);
        subject.getMuscleNames(musclesNames_);

        parseExternalLoadsXml(externalLoadsXmlFilename);
    }



    //this will calculate joint reactions
    template <typename NMSmodelT>
    QueuesToContactForces::QueuesToContactForces(
        GeneralisedCoordinatesQueue& inputGeneralisedCoordinatesQueue,
        ExternalForcesQueue& inputExternalForcesQueue,
        MuscleForcesQueue& inputMuscleForcesQueue,
        EstimatedTorquesQueue& inputEstimatedTorquesQueue,
        ContactForcesQueue& outputJointReactionForcesQueue,
        Concurrency::Latch& latch,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,    //this is the same as openSim
        const std::vector< std::string >& jointsToLog) :
        inputGeneralisedCoordinatesQueue_(inputGeneralisedCoordinatesQueue),
        inputExternalForcesQueue_(inputExternalForcesQueue),
        inputEstimatedTorquesQueue_(inputEstimatedTorquesQueue),
        outputJointReactionForcesQueue_(outputJointReactionForcesQueue),
        latch_(latch),
        modelFilename_(osimModelFilename),
        jointsToLog_(jointsToLog),
        calculateJointReactions_(true),
        calculateIntersegmentalForces_(false),
        useEstimatedTorques_(true) {

        subject.getDoFNames(dofNames_);
        subject.getMuscleNames(musclesNames_);

        parseExternalLoadsXml(externalLoadsXmlFilename);
    }



    //this will calculate joint reactions and intersegmental forces
    template <typename NMSmodelT>
    QueuesToContactForces::QueuesToContactForces(
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
        const std::vector< std::string >& jointsToLog) :
        inputGeneralisedCoordinatesQueue_(inputGeneralisedCoordinatesQueue),
        inputExternalForcesQueue_(inputExternalForcesQueue),
        inputEstimatedTorquesQueue_(inputEstimatedTorquesQueue),
        outputJointReactionForcesQueue_(outputJointReactionForcesQueue),
        outputIntersegmentalForcesQueue_(outputIntersegmentalForcesQueue),
        latch_(latch),
        modelFilename_(osimModelFilename),
        jointsToLog_(jointsToLog),
        calculateJointReactions_(true),
        calculateIntersegmentalForces_(true),
        useEstimatedTorques_(true) {

        subject.getDoFNames(dofNames_);
        subject.getMuscleNames(musclesNames_);

        parseExternalLoadsXml(externalLoadsXmlFilename);
    }


    void QueuesToContactForces::parseExternalLoadsXml(const std::string& xmlFilename) {
        OpenSim::XMLDocument document(xmlFilename);
        SimTK::Xml::Element root = document.getRootDataElement();
        SimTK::Xml::element_iterator extForceElementIter = root.element_begin("objects")->element_begin("ExternalForce");
        
        for (extForceElementIter; extForceElementIter != root.element_begin("objects")->element_end(); ++extForceElementIter) {
            singleExternalForceQueues_.emplace_back(new SingleExternalForceQueue());
            extForces_.push_back(new OpenSim::ExternalForceFromQueue(singleExternalForceQueues_.back(), *extForceElementIter));
        }
    }


    void QueuesToContactForces::operator()() {

        //0. subscriptions to queues
        if (useEstimatedTorques_) inputEstimatedTorquesQueue_.subscribe();
        if (calculateJointReactions_) inputMuscleForcesQueue_.subscribe();

        inputGeneralisedCoordinatesQueue_.subscribe();

        //we create an internal latch, so who is outside doesn't have to bother to increase the count..
        Concurrency::Latch forceSplitLatch(2);
        //1. Divide external forces in different queues
        auto forceDistributorThr(std::async(std::launch::async, [&](){
            
            inputExternalForcesQueue_.subscribe();
            forceSplitLatch.wait();

            bool runCondition(true);
            do {
                QueueData<std::vector<ExternalForceData>> forces(inputExternalForcesQueue_.pop());
                if (forces.time < std::numeric_limits<double>::infinity()) {
                    for (size_t i(0); i < forces.data.size(); ++i) {
                        QueueData<ExternalForceData> currentForce;
                        currentForce.time = forces.time;
                        currentForce.data = forces.data.at(i);
                        singleExternalForceQueues_.at(i)->push(currentForce);
                    }
                }
                else {
                    runCondition = false;
                    QueueData<ExternalForceData> eof;
                    eof.time = std::numeric_limits<double>::infinity();
                    for (auto& forceQueue : singleExternalForceQueues_)
                        forceQueue->push(eof);
                }
            } while (runCondition); 
        }));


        JointContactFromOpensim jointContactModel(modelFilename_, extForces_);
        std::vector<std::string> osimJointNames, existingJointsToLog;
        jointContactModel.getJointNamesFromOsimModel(osimJointNames);
        for (auto& jointName : jointsToLog_) {
            auto it(std::find(osimJointNames.begin(), osimJointNames.end(), jointName));
            if (it != osimJointNames.end())
                existingJointsToLog.emplace_back(*it);
            else
                std::cout << jointName << " does not exist." << std::endl;
        }
        //2. call the latch, say that we are done with initialisations
        latch_.wait();
        forceSplitLatch.wait();

        syncFrames(inputGeneralisedCoordinatesQueue_, inputEstimatedTorquesQueue_);
        bool runCondition(true);
        while (runCondition) {
            GeneralisedCoordinatesFrame generalisedCoordinatesFrame(inputGeneralisedCoordinatesQueue_.pop());

            if (generalisedCoordinatesFrame.time == std::numeric_limits<double>::infinity())
                runCondition = false;
            if (runCondition) {
                jointContactModel.setTime(generalisedCoordinatesFrame.time);
                jointContactModel.setQ(generalisedCoordinatesFrame.data.getQ());
                jointContactModel.setQd(generalisedCoordinatesFrame.data.getQd());
                jointContactModel.setQdd(generalisedCoordinatesFrame.data.getQdd());
                jointContactModel.calculateReactionForces(true);
                ContactForcesFrame contactForces;
                contactForces.time = generalisedCoordinatesFrame.time;
                for (auto& joint : existingJointsToLog) {
                    SimTK::Vec3 reactionForce;
                    jointContactModel.getReactionForceInG(joint, reactionForce);
                    contactForces.data.push_back(reactionForce);

              //      oF << state.time << "\t";
              //      oF << reactionForce[0] << "\t" <<
              //          reactionForce[1] << "\t" <<
              //          reactionForce[2] << "\t";
                }
//                outputContactForcesQueue_.push(contactForces);
             //   oF << std::endl;
            }
        }
        ContactForcesFrame contactForces;
        contactForces.time = std::numeric_limits<double>::infinity();
     //   outputContactForcesQueue_.push(contactForces);

        forceDistributorThr.wait();
    }
}