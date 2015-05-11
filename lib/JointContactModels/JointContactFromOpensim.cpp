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

#include "ceinms/JointContactFromOpensim.h"
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

#include "ceinms/ArrayConverter.h"

namespace ceinms {
 
    JointContactFromOpensim::JointContactFromOpensim(const std::string& modelFilename, std::vector<OpenSim::Force*> externalForces) :
        model_(modelFilename)
        {
        using namespace OpenSim;

        
        Model modelCopy(modelFilename);
        modelCopy.initSystem();
        nCoordinates_ = modelCopy.getNumCoordinates();
        nBodies_ = modelCopy.getNumBodies();
        nMuscles_ = modelCopy.getMuscles().getSize();
        rForcesInG_.resize(nBodies_);
        rTorquesInG_.resize(nBodies_);

        //1. add extForces to the model, hopefully I won't have problems with queues and opensim
        for (auto &force : externalForces) {
            cout << "models add force\n" << std::flush;
            model_.addForce(force);
        }


        OpenSim::Array<string> muscleNames;
        model_.getMuscles().getNames(muscleNames);
        for (unsigned i(0); i < nMuscles_; ++i) 
            muscleActuators_.push_back(dynamic_cast<Actuator_*>(&model_.updForceSet().get(muscleNames.get(i))));

        const CoordinateSet& cs = model_.getCoordinateSet();
        for (unsigned i(0); i < nCoordinates_; ++i) {
            CoordinateActuator *actuator = new CoordinateActuator();
            actuator->setCoordinate(&cs.get(i));
            string actuatorName(cs.get(i).getName() + "_actuator");
            actuator->setName(actuatorName);
            actuator->setOptimalForce(1);
            actuator->setMaxControl(100000);
            actuator->setMinControl(-100000);
            model_.addForce(actuator);
            coordinateActuators_.push_back(dynamic_cast<CoordinateActuator*>(&(model_.updForceSet().get(actuatorName))));
        }
       
        std::unique_ptr<InverseDynamicsSolver> p(new InverseDynamicsSolver(model_));
        idSolver_ = std::move(p);

        state_ = model_.initSystem();
        for (auto c : coordinateActuators_)
            c->overrideForce(state_, true);

        for (auto m : muscleActuators_) {
            m->setDisabled(state_, true);
            m->set_isDisabled(true);
        }
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


    void JointContactFromOpensim::getReactionForce(const std::string& jointName, SimTK::Vec3& forceOnChildInChild) {

        const OpenSim::Joint& joint(model_.getJointSet().get(jointName));
        unsigned idx(model_.getJointSet().getIndex(joint.getName()));
        model_.getSimbodyEngine().transform(state_,
                                            model_.getGroundBody(),
                                            rForcesInG_.get(idx),
                                            joint.getBody(),
                                            forceOnChildInChild);
    }


    void JointContactFromOpensim::getReactionForceInG(const std::string& jointName, SimTK::Vec3& forceOnChildInG) {

        const OpenSim::Joint& joint(model_.getJointSet().get(jointName));
       unsigned idx(model_.getJointSet().getIndex(joint.getName()));
        forceOnChildInG = rForcesInG_.get(idx);
    }


    string JointContactFromOpensim::getJointNameOnDof(const string& dofName) {

        if (model_.getCoordinateSet().getIndex(dofName) < 0) {
            cout << "Error " + dofName + " not found\n";
            std::exit(EXIT_FAILURE);
        }
        return model_.getCoordinateSet().get(dofName).getJoint().getName();

    }


    //if appliedMobilityForces is 0 then it is the ID
    void JointContactFromOpensim::calculateResidualMobilityForces(const SimTK::Vector& appliedMobilityForces) {

        for (auto m : muscleActuators_)
            m->setDisabled(state_, true);        
        //realize the system, so all the forces as gravitational, inertial, and external forces are applied
        model_.getMultibodySystem().realize(state_, SimTK::Stage::Dynamics);
        SimTK::Vector_<SimTK::SpatialVec> appliedBodyForces(model_.getMultibodySystem().getRigidBodyForces(state_, SimTK::Stage::Dynamics));
        residualMobilityForces_ = idSolver_->solve(state_, state_.getUDot(), appliedMobilityForces, appliedBodyForces);
    }

    //estimatedMobilityForces is the torque predicted by CEINMS
    //if disableMuscles is true, then intersegmental forces are calculated
    void JointContactFromOpensim::calculateReactionForcesWithExternalEstimatedAndResidualMobilityForces(const SimTK::Vector& estimatedMobilityForces, const SimTK::Vector& residualMobilityForces, bool disableMuscles) {

        for (auto m : muscleActuators_)
            m->setDisabled(state_, disableMuscles);
        for (unsigned i(0); i < nCoordinates_; ++i)
            coordinateActuators_.at(i)->setOverrideForce(state_, residualMobilityForces.get(i) - estimatedMobilityForces.get(i));
        model_.getSimbodyEngine().computeReactions(state_, rForcesInG_, rTorquesInG_);
    }

    // residualMobilityForces is ID from external file
    //if disableMuscles is true, then intersegmental forces are calculated
    void JointContactFromOpensim::calculateReactionForcesWithExternalResidualMobilityForces(const SimTK::Vector& residualMobilityForces, bool disableMuscles) {

        for (auto m : muscleActuators_)
            m->setDisabled(state_, disableMuscles);
        for (unsigned i(0); i < nCoordinates_; ++i)
            coordinateActuators_.at(i)->setOverrideForce(state_, residualMobilityForces.get(i));
        model_.getSimbodyEngine().computeReactions(state_, rForcesInG_, rTorquesInG_);
    }

    //estimatedMobilityForces is the torque predicted by CEINMS
    //if disableMuscles is true, then intersegmental forces are calculated
    void JointContactFromOpensim::calculateReactionForcesWithExternalEstimatedMobilityForces(const SimTK::Vector& estimatedMobilityForces, bool disableMuscles) {

        SimTK::Vector appliedMobilityForces(nCoordinates_, 0.); //Calculate ID
        calculateResidualMobilityForces(appliedMobilityForces);
        calculateReactionForcesWithExternalEstimatedAndResidualMobilityForces(estimatedMobilityForces, residualMobilityForces_, disableMuscles);
    }


    //if disableMuscles is true, then intersegmental forces are calculated
    //this also computes ID
    void JointContactFromOpensim::calculateReactionForces(bool disableMuscles) {

        SimTK::Vector appliedMobilityForces(nCoordinates_, 0.); //Calculate ID
        calculateResidualMobilityForces(appliedMobilityForces);
       
        for (auto m : muscleActuators_)
            m->setDisabled(state_, disableMuscles);
        for (unsigned i(0); i < nCoordinates_; ++i)
            coordinateActuators_.at(i)->setOverrideForce(state_, residualMobilityForces_.get(i));
        model_.getSimbodyEngine().computeReactions(state_, rForcesInG_, rTorquesInG_);
    }


    void JointContactFromOpensim::getAllReactionForces(SimTK::Vector_<SimTK::Vec3>& reactionForces, SimTK::Vector_<SimTK::Vec3>& reactionTorques) const {
        reactionForces = rForcesInG_;
        reactionTorques = rTorquesInG_;
    }
}
