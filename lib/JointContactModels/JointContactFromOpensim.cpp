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

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

#include <OpenSim/Tools/DynamicsTool.h>

#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/FileUtils.h"

namespace ceinms {

    JointContactFromOpensim::JointContactFromOpensim(const string& modelFilename, vector<OpenSim::Force*> externalForces) :
        model_(modelFilename),
        idSolver_(nullptr) {

        OpenSim::Model modelCopy(modelFilename);
        modelCopy.initSystem();
        nCoordinates_ = modelCopy.getNumCoordinates();
        nBodies_ = modelCopy.getNumBodies();
        nMuscles_ = modelCopy.getMuscles().getSize();
        // computeReactions() asserts its output vectors are sized exactly to
        // the number of joints (its output is indexed by JointSet order, not
        // by body count -- see getReactionForceInG()).
        rForcesInG_.resize(modelCopy.getNumJoints());
        rTorquesInG_.resize(modelCopy.getNumJoints());

        qdd_.resize(nCoordinates_);

        //1. add extForces to the model, hopefully I won't have problems with queues and opensim
        for (auto &force : externalForces)
            model_.addForce(force);

        model_.getMuscles().getNames(muscleActuatorNames_);
        for (unsigned i(0); i < nMuscles_; ++i)
            muscleActuators_.push_back((const_cast<OpenSim::Muscle*>(&model_.updForceSet().getMuscles().get(muscleActuatorNames_.get(i)))));

        const OpenSim::CoordinateSet& cs = model_.getCoordinateSet();
        for (unsigned i(0); i < nCoordinates_; ++i) {
            OpenSim::CoordinateActuator *actuator = new OpenSim::CoordinateActuator();
            actuator->setCoordinate(&cs.get(i));
            string actuatorName(cs.get(i).getName() + "_actuator_ceinms");
            actuator->setName(actuatorName);
            actuator->setOptimalForce(10000);
            actuator->setMaxControl(100000);
            actuator->setMinControl(-100000);
            model_.addForce(actuator);
            coordinateActuators_.push_back(dynamic_cast<OpenSim::CoordinateActuator*>(&(model_.updForceSet().get(actuatorName))));
        }

        idSolver_ = new OpenSim::InverseDynamicsSolver(model_);

        state_ = model_.initSystem();
        for (auto c : coordinateActuators_)
            c->overrideActuation(state_, true);
        for (auto m : muscleActuators_)
            m->overrideActuation(state_, true);

        buildQIndexMap();
    }

    JointContactFromOpensim::JointContactFromOpensim(const string& modelFilename, string externalLoadXmlFilename) :
        model_(modelFilename) {
            
        OpenSim::Model modelCopy(modelFilename);
        modelCopy.initSystem();
        nCoordinates_ = modelCopy.getNumCoordinates();
        nBodies_ = modelCopy.getNumBodies();
        nMuscles_ = modelCopy.getMuscles().getSize();
        rForcesInG_.resize(modelCopy.getNumJoints());
        rTorquesInG_.resize(modelCopy.getNumJoints());
        qdd_.resize(nCoordinates_);

        OpenSim::ExternalLoads* extLoads(new OpenSim::ExternalLoads(externalLoadXmlFilename, 1));

        string baseDir(FileUtils::getFilePath(externalLoadXmlFilename));

        string dataFilename = extLoads->getDataFileName();
        if (!FileUtils::isAbsolute(dataFilename.c_str())) {
            dataFilename = FileUtils::simplify(baseDir + '/' + dataFilename);
            extLoads->setDataFileName(dataFilename);
        }
        extLoads->setMemoryOwner(false);
        extLoads->extendConnectToModel(model_);
        // Add external loads to the set of all model forces
        for (int i = 0; i<extLoads->getSize(); ++i) {
            model_.addForce(&extLoads->get(i));
        }

        model_.getMuscles().getNames(muscleActuatorNames_);
        for (unsigned i(0); i < nMuscles_; ++i)
            muscleActuators_.push_back((dynamic_cast<OpenSim::Muscle*>(&model_.updForceSet().get(muscleActuatorNames_.get(i)))));

        const OpenSim::CoordinateSet& cs = model_.getCoordinateSet();
        for (unsigned i(0); i < nCoordinates_; ++i) {
            OpenSim::CoordinateActuator *actuator = new OpenSim::CoordinateActuator();
            actuator->setCoordinate(&cs.get(i));
            string actuatorName(cs.get(i).getName() + "_actuator_ceinms");
            actuator->setName(actuatorName);
            actuator->setOptimalForce(10000);
            actuator->setMaxControl(100000);
            actuator->setMinControl(-100000);
            model_.addForce(actuator);
            coordinateActuators_.push_back(dynamic_cast<OpenSim::CoordinateActuator*>(&(model_.updForceSet().get(actuatorName))));
        }
        idSolver_ = new OpenSim::InverseDynamicsSolver(model_);

        state_ = model_.initSystem();
        for (auto c : coordinateActuators_)
            c->overrideActuation(state_, true);
        for (auto m : muscleActuators_)
            m->overrideActuation(state_, true);

        buildQIndexMap();

        OpenSim::Array<string> names;
        model_.getForceSet().getNames(names);
        for (int i(0); i < names.size(); ++i)
            cout << names[i] << endl;
    }

    void JointContactFromOpensim::buildQIndexMap() {
        // CoordinateSet order (file/subject-DOF order) does not generally
        // match a coordinate's actual index in the state's q/u/udot vectors,
        // which Simbody assigns by multibody-tree order. Resolve each
        // coordinate's true index once, via its owning MobilizedBody, rather
        // than assuming CoordinateSet index i lands at state vector index i.
        const OpenSim::CoordinateSet& cs = model_.getCoordinateSet();
        qIndexMap_.resize(nCoordinates_);
        uIndexMap_.resize(nCoordinates_);
        for (unsigned i(0); i < nCoordinates_; ++i) {
            const OpenSim::Coordinate& c = cs.get(i);
            const SimTK::MobilizedBody& mb = model_.getMatterSubsystem().getMobilizedBody(c.getBodyIndex());
            qIndexMap_[i] = (int)mb.getFirstQIndex(state_) + c.getMobilizerQIndex();
            uIndexMap_[i] = (int)mb.getFirstUIndex(state_) + c.getMobilizerQIndex();
        }
    }

    void JointContactFromOpensim::setTime(double time) {
        state_.updTime() = time;
    }

    void JointContactFromOpensim::getResidualMobilityForces(SimTK::Vector& residualMobilityForces) const {
        residualMobilityForces = residualMobilityForces_;
    }

    void JointContactFromOpensim::getMuscleNamesFromOsimModel(vector<string>& names) const {
        OpenSim::Array<string> namesOsim;
        model_.getMuscles().getNames(namesOsim);
        ArrayConverter::toStdVector(namesOsim, names);
    }

    void JointContactFromOpensim::getCoordinateNamesFromOsimModel(vector<string>& names) const {
        OpenSim::Array<string> namesOsim;
        model_.getCoordinateSet().getNames(namesOsim);
        ArrayConverter::toStdVector(namesOsim, names);
    }

    void JointContactFromOpensim::getBodyNamesFromOsimModel(vector<string>& names) const {
        OpenSim::Array<string> namesOsim;
        model_.getBodySet().getNames(namesOsim);
        ArrayConverter::toStdVector(namesOsim, names);
    }

    void JointContactFromOpensim::getJointNamesFromOsimModel(vector<string>& names) const {
        OpenSim::Array<string> namesOsim;
        model_.getJointSet().getNames(namesOsim);
        ArrayConverter::toStdVector(namesOsim, names);
    }

    void JointContactFromOpensim::getJointReactionForceInG(const string& jointName, SimTK::Vec3& forceOnChildInG) const {
        getReactionForceInG(jrForcesInG_, jointName, forceOnChildInG);
    }

    void JointContactFromOpensim::getJointReactionForceInChild(const string& jointName, SimTK::Vec3& forceOnChildInChild) const {
        getReactionForceInChild(jrForcesInG_, jointName, forceOnChildInChild);
    }

    void JointContactFromOpensim::getIntersegmentalForceInG(const string& jointName, SimTK::Vec3& forceOnChildInG) const {
        getReactionForceInG(intForcesInG_, jointName, forceOnChildInG);
    }

    void JointContactFromOpensim::getIntersegmentalForceInChild(const string& jointName, SimTK::Vec3& forceOnChildInChild) const {
        getReactionForceInChild(intForcesInG_, jointName, forceOnChildInChild);
    }

    void JointContactFromOpensim::getReactionForceInChild(const SimTK::Vector_<SimTK::Vec3>& rForcesInG, const string& jointName, SimTK::Vec3& forceOnChildInChild) const {
        const OpenSim::Joint& joint(model_.getJointSet().get(jointName));
        unsigned idx(model_.getJointSet().getIndex(joint.getName()));
        forceOnChildInChild = model_.getGround().expressVectorInAnotherFrame(state_, rForcesInG.get(idx), joint.getChildFrame());
    }

    void JointContactFromOpensim::getReactionForceInG(const SimTK::Vector_<SimTK::Vec3>& rForcesInG, const string& jointName, SimTK::Vec3& forceOnChildInG)  const {
        // OpenSim::SimbodyEngine::computeReactions() already resolves each
        // joint's MobilizedBodyIndex internally and writes its output indexed
        // by plain JointSet order (rForces[i] <-> joints[i]), NOT by
        // MobilizedBodyIndex -- so the lookup here must use the same plain
        // JointSet index, matching getReactionForceInChild() below.
        unsigned idx(model_.getJointSet().getIndex(jointName));
        forceOnChildInG = rForcesInG.get(idx);
    }

    string JointContactFromOpensim::getJointNameOnDof(const string& dofName) {
        if (model_.getCoordinateSet().getIndex(dofName) < 0) {
            cout << "Error " + dofName + " not found\n";
            std::exit(EXIT_FAILURE);
        }
        return model_.getCoordinateSet().get(dofName).getJoint().getName();
    }

    void JointContactFromOpensim::calculateIntersegmentalForces() {
        calculateInverseDynamics();
        disableMuscleActuators();
        calculateReactions();
        intForcesInG_ = rForcesInG_;
    }

    void JointContactFromOpensim::calculateIntersegmentalForcesUsingExternalMobilityForces(const SimTK::Vector& realMobilityForces) {
        residualMobilityForces_ = realMobilityForces;
        disableMuscleActuators();
        calculateReactions();
        intForcesInG_ = rForcesInG_;
    }

    void JointContactFromOpensim::calculateJointReactionForces(const SimTK::Vector& muscleForces, const SimTK::Vector& estimatedMobilityForces) {
        calculateInverseDynamics();
        enableMuscleActuators();
        setMuscleForcesToActuators(muscleForces);
        SimTK::Vector balancingResidual(residualMobilityForces_ - estimatedMobilityForces);
        calculateReactions(balancingResidual);
        jrForcesInG_ = rForcesInG_;
    }

    void JointContactFromOpensim::calculateJointReactionForcesUsingExternalMobilityForces(const SimTK::Vector& muscleForces, const SimTK::Vector& estimatedMobilityForces, const SimTK::Vector& realMobilityForces) {
        enableMuscleActuators();
        setMuscleForcesToActuators(muscleForces);
        calculateReactions(realMobilityForces - estimatedMobilityForces);
        jrForcesInG_ = rForcesInG_;
    }

    void JointContactFromOpensim::calculateInverseDynamics() {
        //this updates residualMobilityForces_
        disableMuscleActuators();
        SimTK::Vector appliedMobilityForces(nCoordinates_, 0.); //Calculate ID
        calculateResidualMobilityForces(appliedMobilityForces);
     }

    void JointContactFromOpensim::disableMuscleActuators() {
        for (auto& m : muscleActuators_)
            m->setAppliesForce(state_, false);
    }

    void JointContactFromOpensim::enableMuscleActuators() {
        for (auto& m : muscleActuators_)
            m->setAppliesForce(state_, true);
    }

    void JointContactFromOpensim::setMuscleForcesToActuators(const SimTK::Vector& muscleForces) {
        for (size_t i(0); i < muscleActuators_.size(); ++i) {
            muscleActuators_.at(i)->overrideActuation(state_, true);
            muscleActuators_.at(i)->setOverrideActuation(state_, muscleForces[i]);
        }
    }

    void JointContactFromOpensim::calculateReactions() {
        calculateReactions(residualMobilityForces_);
    }

    void JointContactFromOpensim::calculateReactions(const SimTK::Vector& residualMobilityForces) {
        for (unsigned i(0); i < nCoordinates_; ++i) {
            coordinateActuators_.at(i)->overrideActuation(state_, true);
            coordinateActuators_.at(i)->setOverrideActuation(state_, residualMobilityForces.get(i));
        }
        model_.getSimbodyEngine().computeReactions(state_, rForcesInG_, rTorquesInG_);
    }

    //if appliedMobilityForces is 0 then it is the ID
    void JointContactFromOpensim::calculateResidualMobilityForces(const SimTK::Vector& appliedMobilityForces) {
        //realize the system, so all the forces as gravitational, inertial, and external forces are applied
        model_.realizeDynamics(state_);
        SimTK::Vector_<SimTK::SpatialVec> appliedBodyForces(model_.getMultibodySystem().getRigidBodyForces(state_, SimTK::Stage::Dynamics));

        // qdd_ and appliedMobilityForces are in CoordinateSet order; the
        // solver needs them in the state's true global u/udot order.
        SimTK::Vector qddGlobal(state_.getNU(), 0.0);
        SimTK::Vector appliedMobilityForcesGlobal(state_.getNU(), 0.0);
        for (unsigned i(0); i < nCoordinates_; ++i) {
            qddGlobal[uIndexMap_[i]] = qdd_[i];
            appliedMobilityForcesGlobal[uIndexMap_[i]] = appliedMobilityForces[i];
        }

        SimTK::Vector residualGlobal(idSolver_->solve(state_, qddGlobal, appliedMobilityForcesGlobal, appliedBodyForces));

        residualMobilityForces_.resize(nCoordinates_);
        for (unsigned i(0); i < nCoordinates_; ++i)
            residualMobilityForces_[i] = residualGlobal[uIndexMap_[i]];
    }

    void JointContactFromOpensim::getAllReactionForces(SimTK::Vector_<SimTK::Vec3>& reactionForces, SimTK::Vector_<SimTK::Vec3>& reactionTorques) const {
        // rForcesInG_/rTorquesInG_ are already indexed by plain JointSet
        // order (see getReactionForceInG()), so this is a direct copy.
        const OpenSim::JointSet& joints(model_.getJointSet());
        reactionForces.resize(joints.getSize());
        reactionTorques.resize(joints.getSize());
        for (int i(0); i < joints.getSize(); ++i) {
            reactionForces[i] = rForcesInG_.get(i);
            reactionTorques[i] = rTorquesInG_.get(i);
        }
    }

    JointContactFromOpensim::~JointContactFromOpensim() {
        // if (idSolver_ != nullptr)
        //     delete idSolver_;
    }
}