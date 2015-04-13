#include "JointContactFromOpensim.h"
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

#include "ArrayConverter.h"


namespace CEINMS {
 
    JointContactFromOpensim::JointContactFromOpensim(const OpenSim::Model& model) :
        model_(model),
        nCoordinates_(model_.getNumCoordinates()),
        nBodies_(model_.getNumBodies()),
        nMuscles_(model_.getMuscles().getSize()),
        rForcesInG_(nBodies_), rTorquesInG_(nBodies_)
        {
        using namespace OpenSim;

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

  /*      OpenSim::Array<string> bodyNames;

        model_.getBodySet().getNames(bodyNames);
        cout << bodyNames << endl << std::flush;

        OpenSim::Array<string> actNames;

        model_.updForceSet().getNames(actNames);
        cout << actNames << endl << std::flush;
        */

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



    void JointContactFromOpensim::setEstimatedMobilityForces(const SimTK::Vector& estimatedMobilityForces) {

        estimatedMobilityForces_  = estimatedMobilityForces;
    }


    void JointContactFromOpensim::setResidualMobilityForces(const SimTK::Vector& residualMobilityForces) {

        residualMobilityForces_ = residualMobilityForces;
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


    //if disableMuscles is true, then intersegmental forces are calculated
    void JointContactFromOpensim::calculateReactionForcesUsingEstimatedMobilityForces(bool disableMuscles) {

        for (auto m : muscleActuators_)
            m->setDisabled(state_, disableMuscles);

        for (unsigned i(0); i < nCoordinates_; ++i)
            coordinateActuators_.at(i)->setOverrideForce(state_, residualMobilityForces_.get(i)-estimatedMobilityForces_.get(i));
        model_.getSimbodyEngine().computeReactions(state_, rForcesInG_, rTorquesInG_);
    }


    //if disableMuscles is true, then intersegmental forces are calculated
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
