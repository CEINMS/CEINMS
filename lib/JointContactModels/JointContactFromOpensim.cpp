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



    void JointContactFromOpensim::setEstimatedJointMoments(const vector<double>& estimatedJointMoments) {

        estimatedJointMoments_.assign(estimatedJointMoments.cbegin(), estimatedJointMoments.cend());
    }
    

    void JointContactFromOpensim::getReactionForce(const std::string& childBodyName, SimTK::Vec3& forceOnChildInChild) {

        const OpenSim::Joint& joint(model_.getBodySet().get(childBodyName).getJoint());
        cout << "joint name: " << joint.getName() << endl;
        unsigned idx(model_.getJointSet().getIndex(joint.getName()));
        model_.getSimbodyEngine().transform(state_,
                                            model_.getGroundBody(),
                                            rForcesInG_.get(idx),
                                            model_.getBodySet().get(childBodyName),
                                            forceOnChildInChild);
    }


    void JointContactFromOpensim::getReactionForceInG(const std::string& childBodyName, SimTK::Vec3& forceOnChildInG) {

        const OpenSim::Joint& joint(model_.getBodySet().get(childBodyName).getJoint());
        cout << "joint name: " << joint.getName() << endl;
        unsigned idx(model_.getJointSet().getIndex(joint.getName()));
        forceOnChildInG = rForcesInG_.get(idx);
    }


    void JointContactFromOpensim::calculateReactionForces() {


    }


    void JointContactFromOpensim::calculateIntersegmentalForces() {

        for (auto m : muscleActuators_)
            m->setDisabled(state_, true);
        //realize the system, so all the forces as gravitational, inertial, and external forces are applied
        model_.getMultibodySystem().realize(state_, SimTK::Stage::Dynamics);
        SimTK::Vector_<SimTK::SpatialVec> appliedBodyForces(model_.getMultibodySystem().getRigidBodyForces(state_, SimTK::Stage::Dynamics));
        SimTK::Vector appliedMobilityForces(nCoordinates_, 0.);
  
        residualMobilityForces_ = idSolver_->solve(state_, state_.getUDot(), appliedMobilityForces, appliedBodyForces);
        for (unsigned i(0); i < nCoordinates_; ++i)
            coordinateActuators_.at(i)->setOverrideForce(state_, residualMobilityForces_.get(i));
        model_.getSimbodyEngine().computeReactions(state_, rForcesInG_, rTorquesInG_);
    }


    void JointContactFromOpensim::getAllReactionForces(SimTK::Vector_<SimTK::Vec3>& reactionForces, SimTK::Vector_<SimTK::Vec3>& reactionTorques) const {
        reactionForces = rForcesInG_;
        reactionTorques = rTorquesInG_;
    }


}
