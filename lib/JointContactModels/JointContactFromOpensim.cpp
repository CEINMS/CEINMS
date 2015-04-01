#include "JointContactFromOpensim.h"
#include <vector>
using std::vector;
#include <string>
using std::string;

#include "ArrayConverter.h"

namespace CEINMS {
 
    JointContactFromOpensim::JointContactFromOpensim(const OpenSim::Model& model) :
        model_(model),
        nCoordinates_(model_.getNumCoordinates()),
        nBodies_(model_.getBodySet().getSize()),
        nMuscles_(model_.getMuscles().getSize()),
        state_(model_.initializeState()) {        

        //blank..
       
    }

    void JointContactFromOpensim::setQ(const vector<double>& q) {
        for (size_t i(0); i < nCoordinates_; ++i) {
            state_.updQ()[i] = q.at(i);
        }
    }


    void JointContactFromOpensim::setQd(const vector<double>& qd) {
        for (size_t i(0); i < nCoordinates_; ++i) {
            state_.updQDot()[i] = qd.at(i);
        }
    }


    void JointContactFromOpensim::setQdd(const vector<double>& qdd) {
        for (size_t i(0); i < nCoordinates_; ++i) {
            state_.updQDotDot()[i] = qdd.at(i);
        }
    }


    void JointContactFromOpensim::addExternalForceToMobilizedBodyPoint(const ExternalForce& externalForce) {

        externalForces_.push_front(externalForce);
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
    

    void JointContactFromOpensim::calculateReactionForces() {

        //1 create the appropriate external force vector
        SimTK::Vector_<SimTK::SpatialVec> appliedBodyForces(nBodies_, SimTK::SpatialVec(SimTK::Vec3(.0), model_.getGravity()));
        applyExternalForces(appliedBodyForces);

        //2 disable muscle forces
        for (size_t i(0); i < nMuscles_; ++i)
            model_.updForceSet().get(i).setDisabled(s, true);

        //3 calculate ID
        model_.getMultibodySystem().realize(s, SimTK::Stage::Velocity);
        SimTK::Vector residualMobilityForces;
        model_.getMultibodySystem().getMatterSubsystem().calcResidualForceIgnoringConstraints(state_, SimTK::Vector(), appliedBodyForces, state_.updQDotDot(), residualMobilityForces);

        //4 apply id 



    }

    void JointContactFromOpensim::calculateIntersegmentalForces() {

        //1 create the appropriate external force vector
        SimTK::Vector_<SimTK::SpatialVec> appliedBodyForces(nBodies_, SimTK::SpatialVec(SimTK::Vec3(.0), model_.getGravity()));
        applyExternalForces(appliedBodyForces);

        //2 disable muscle forces
        for (size_t i(0); i < nMuscles_; ++i)
            model_.updForceSet().get(i).setDisabled(state_, true);

        //3 calculate ID
        model_.getMultibodySystem().realize(state_, SimTK::Stage::Velocity);
        SimTK::Vector residualMobilityForces;
        model_.getMultibodySystem().getMatterSubsystem().calcResidualForceIgnoringConstraints(state_, SimTK::Vector(), appliedBodyForces, state_.updQDotDot(), residualMobilityForces);

        //4 transform torques in ground reference
        
        //5 apply id results to model 
        //5.1 calculate torques to bodies
    //    model_.getMultibodySystem().getMatterSubsystem().addInBodyTorque()
        //5.2 apply body torques to the system
//        model_.getMultibodySystem().getMatterSubsystem().getMobilizedBody(index).applyBodyTorque();
        //5.3 calculate reaction forces/intersegmental forces
        //      model_.getMultibodySystem().getMatterSubsystem().calcMobilizerReactionForces


    }

}
