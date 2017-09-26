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

#include "ceinms/QueuesToXForces.h"
#include "ceinms/JointContactFromOpensim.h" 
#include <limits>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <SimTKcommon.h>
using SimTK::Vec3;
using ceinms::SingleExternalForceFrame;
using ceinms::SingleExternalForceQueue;

namespace OpenSim {

    ExternalForceFromQueue::ExternalForceFromQueue
        (ceinms::SingleExternalForceQueue* externalForceQueue,
        const std::string& forceIdentifier,
        const std::string& pointIdentifier,
        const std::string& torqueIdentifier,
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
        setForceIdentifier(forceIdentifier);
        setPointIdentifier(pointIdentifier);
        setTorqueIdentifier(torqueIdentifier);
        setAppliedToBodyName(appliedToBodyName);
        setForceExpressedInBodyName(forceExpressedInBodyName);
        setPointExpressedInBodyName(pointExpressedInBodyName);
    }

    ExternalForceFromQueue::ExternalForceFromQueue(ceinms::SingleExternalForceQueue* externalForceQueue, SimTK::Xml::Element& node) :
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

        SingleExternalForceFrame forceFrame;
        while (time > lastTime_ && runCondition_){
            forceFrame = externalForceQueue_->pop();
            if (forceFrame.time == std::numeric_limits<double>::infinity()) {
                runCondition_ = false;
                forceFrame.data.setForceVector(SimTK::Vec3(0.));
                forceFrame.data.setApplicationPoint(SimTK::Vec3(0.));
                forceFrame.data.setTorque(SimTK::Vec3(0.));
            }
            if (runCondition_) {
                pastExternalForceFrames_.push_back(forceFrame);
                lastTime_ = forceFrame.time;
                if (pastExternalForceFrames_.size() > MaxStorageLength_)
                    pastExternalForceFrames_.pop_front();
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

            SimTK::Vec3 torquesToApply(0.);

            const SimbodyEngine& engine = getModel().getSimbodyEngine();
            SimTK::Vec3 transformedForce, transformedPosition, transformedTorque;
            engine.transform(state, *forceExpressedInBody_, forceData.getForce(), engine.getGroundBody(), transformedForce);

            if (forceData.getUseApplicationPoint()) {
                //use COP + GRF + fre torque
                torquesToApply = forceData.getTorque();
                engine.transformPosition(state, *pointExpressedInBody_, forceData.getApplicationPoint(), *appliedToBody_, transformedPosition);
            }
            else {
                //apply the grf and moments from the force plate directly to the body
                pointExpressedInBody_->getMassCenter(transformedPosition);
                torquesToApply = forceData.getMoments();
            }

            ExternalForce::applyForceToPoint(state, *appliedToBody_, transformedPosition, transformedForce, bodyForces);

            engine.transform(state, *forceExpressedInBody_, torquesToApply, engine.getGroundBody(), transformedTorque);
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

        if (externalForceQueue_ != nullptr)
            delete externalForceQueue_;

        //     externalForceQueue_.unsubscribe();
    }
}

namespace ceinms {

    void QueuesToXForces::initialiseContactModel() {

        initialiseExternalForces();

        //Using a pointer to delay the initialization
        jointContactModel_.reset(new JointContactFromOpensim(osimModelFilename_, extForces_));
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

    void QueuesToXForces::initialiseExternalForces() {

        for (auto& it : externalForcesDataSetups_) {

            singleExternalForceQueues_.emplace_back(new SingleExternalForceQueue());
            extForces_.push_back(new OpenSim::ExternalForceFromQueue(
                singleExternalForceQueues_.back(),
                it.forceIdentifier,
                it.pointIdentifier,
                it.torqueIdentifier,
                it.appliedToBodyName,
                it.forceExpressedInBodyName,
                it.pointExpressedInBodyName));
        }
    }

    void QueuesToXForces::distributeExternalForces() {

        //launch the thread, and let it go.. wait iws in the distructor
        splitForcesThread_ = (std::async(std::launch::async, [&](){

            inputExternalForcesQueue_.subscribe();
            splitForcesLatch_.wait();

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
    }

    void QueuesToXForces::doneWithSubscriptions() {

        //from a queue with a vector of N forces we distribute the external forces in N queues..
        //each of them connected internally to an OpenSim model
        distributeExternalForces();
        splitForcesLatch_.wait();
        externalDoneWithSubscriptionsLatch_.wait();
    }

    void QueuesToXForces::setState(double time, const vector<double>& q, const vector<double>& qd, const vector<double>& qdd) {

        jointContactModel_->setTime(time);
        jointContactModel_->setQ(q);
        jointContactModel_->setQd(qd);
        jointContactModel_->setQdd(qdd);
    }

    SimTK::Vector QueuesToXForces::toSimTKVector(std::vector<double> v) {

        SimTK::Vector ansV(v.size(), .0);
        for (size_t i(0); i < v.size(); ++i)
            ansV.set(i, v.at(i));
        return ansV;
    }

    std::vector<double> QueuesToXForces::fromSimTKVector(SimTK::Vector v) {

        std::vector<double> ansV(v.size(), .0);
        for (size_t i(0); i < v.size(); ++i)
            ansV[i] = v[i];
        return ansV;
    }

    void QueuesToXForces::calculateIntersegmentalForces() {

        jointContactModel_->calculateIntersegmentalForces();
    }

    void QueuesToXForces::calculateInverseDynamics() {

        jointContactModel_->calculateInverseDynamics();
    }

    void QueuesToXForces::calculateIntersegmentalForces(const vector<double>& realMobilityForces) {

        //todo put forces and torques in the right place in the vector
        jointContactModel_->calculateIntersegmentalForcesUsingExternalMobilityForces(toSimTKVector(realMobilityForces));
    }

    void QueuesToXForces::calculateJointReactionForces(const vector<double>& muscleForces, const vector<double>& estimatedMobilityForces) {

        //todo put forces and torques in the right place in the vector
        auto forces(toSimTKVector(musclesMapping_.map(muscleForces)));
        auto torques(toSimTKVector(dofsMapping_.map(estimatedMobilityForces)));
        jointContactModel_->calculateJointReactionForces(forces, torques);
    }

    void QueuesToXForces::calculateJointReactionForces(const vector<double>& muscleForces, const vector<double>& estimatedMobilityForces, const vector<double>& realMobilityForces) {

        jointContactModel_->calculateJointReactionForcesUsingExternalMobilityForces(
            toSimTKVector(musclesMapping_.map(muscleForces)),
            toSimTKVector(dofsMapping_.map(estimatedMobilityForces)),
            toSimTKVector(realMobilityForces));
    }

    void QueuesToXForces::getJointReactionForcesInG(vector<Vec3>& jointReactionsInG) const {

        jointReactionsInG.clear();
        for (auto& jointName : jointsToLog_) {
            Vec3 force;
            jointContactModel_->getJointReactionForceInG(jointName, force);
            jointReactionsInG.emplace_back(force);
        }
    }

    void QueuesToXForces::getIntersegmentalForcesInG(vector<Vec3>& intersegmentalForcesInG) const {

        intersegmentalForcesInG.clear();
        for (auto& jointName : jointsToLog_) {
            Vec3 force;
            jointContactModel_->getIntersegmentalForceInG(jointName, force);
            intersegmentalForcesInG.emplace_back(force);
        }
    }

    void QueuesToXForces::getJointReactionForcesInChild(vector<Vec3>& jointReactionsInChild) const {

        jointReactionsInChild.clear();
        for (auto& jointName : jointsToLog_) {
            Vec3 force;
            jointContactModel_->getJointReactionForceInChild(jointName, force);
            jointReactionsInChild.emplace_back(force);
        }
    }

    void QueuesToXForces::getIntersegmentalForcesInChild(vector<Vec3>& intersegmentalForcesInChild) const {

        intersegmentalForcesInChild.clear();
        for (auto& jointName : jointsToLog_) {
            Vec3 force;
            jointContactModel_->getIntersegmentalForceInChild(jointName, force);
            intersegmentalForcesInChild.emplace_back(force);
        }
    }

    void QueuesToXForces::getResidualMobilityForces(vector<double>& residualMobilityForces, bool limitToSubjectDofs) const {

        SimTK::Vector allResidualMobilityForces;
        jointContactModel_->getResidualMobilityForces(allResidualMobilityForces);
        if (limitToSubjectDofs)
            residualMobilityForces = dofsMapping_.map(fromSimTKVector(allResidualMobilityForces));
        else
            residualMobilityForces = fromSimTKVector(allResidualMobilityForces);
    }

    void QueuesToXForces::doneWithExecution() {

        externalDoneWithExecutionLatch_.wait();
    }

    QueuesToXForces::~QueuesToXForces() {

        if (splitForcesThread_.valid())
            splitForcesThread_.wait();
        //     for (OpenSim::Force* it : extForces_)
        //        if (it != nullptr)
        //           delete it;
    }
}
