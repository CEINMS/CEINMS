#include "JointContactFromOpensim.h"
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>

#include "ArrayConverter.h"

namespace CEINMS {
 
    JointContactFromOpensim::JointContactFromOpensim(const OpenSim::Model& model) :
        model_(model),
        nCoordinates_(model_.getNumCoordinates()),
        nBodies_(model_.getBodySet().getSize()),
        nMuscles_(model_.getMuscles().getSize()),
        state_(model_.initializeState()),
        mobilizerForces_(model_.updForceSubsystem(), model_.getMatterSubsystem()),
        reactionForcesInG_(nCoordinates_, SimTK::SpatialVec(SimTK::Vec3(.0), SimTK::Vec3(.0))){

       //blank
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


    void JointContactFromOpensim::addExternalForce(const ExternalForce& externalForce) {

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
    
    void JointContactFromOpensim::applyExternalForces(SimTK::Vector_<SimTK::SpatialVec>& forces) const {
        //precondition: forces has to have size equal to nBodies
        for (auto& f : externalForces_) {
            SimTK::Vec3 pAppliedBodyInB, fAppliedBodyInG;
            //transform position in body and force in ground
            
            model_.getSimbodyEngine().transformPosition(state_,
                                                        model_.getBodySet().get(f.point.first),
                                                        f.point.second,
                                                        model_.getBodySet().get(f.targetBody),
                                                        pAppliedBodyInB);
        
            model_.getSimbodyEngine().transform(state_,
                                                model_.getBodySet().get(f.force.first),
                                                f.force.second,
                                                model_.getGroundBody(),
                                                fAppliedBodyInG);

            SimTK::MobilizedBodyIndex index(f.targetBody);
            model_.getMatterSubsystem().getMobilizedBody(index).applyForceToBodyPoint(state_, 
                                                                                      pAppliedBodyInB, 
                                                                                      fAppliedBodyInG, 
                                                                                      forces);
        }
    }

    void JointContactFromOpensim::calculateReactionForces() {
        std::cout << "calculateReactionForces not tested yet\n";
        exit(EXIT_FAILURE);
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

        //4 apply id 



    }

    void JointContactFromOpensim::calculateID(const VecSPVec& appliedBodyForces, const SimTK::Vector& appliedMobilityForces, SimTK::Vector& residualMobilityForces) {

        model_.getMultibodySystem().realize(state_, SimTK::Stage::Velocity);
        model_.getMatterSubsystem().calcResidualForceIgnoringConstraints(state_, appliedMobilityForces, appliedBodyForces, state_.updQDotDot(), residualMobilityForces);

    }

    void JointContactFromOpensim::calculateIntersegmentalForces() {

        //1 create the appropriate external force vector
        SimTK::Vector_<SimTK::SpatialVec> appliedBodyForces(nBodies_, SimTK::SpatialVec(SimTK::Vec3(.0), model_.getGravity()));
        applyExternalForces(appliedBodyForces);

        //2 disable muscle forces
        for (size_t i(0); i < nMuscles_; ++i)
            model_.updForceSet().get(i).setDisabled(state_, true);

        //3 calculate ID
        SimTK::Vector residualMobilityForces;
        calculateID(appliedBodyForces, SimTK::Vector(), residualMobilityForces);

        //4 apply id results to model 
        //4.1 apply generalised forces to the system
        mobilizerForces_.setAllMobilityForces(state_, residualMobilityForces);
        model_.getMultibodySystem().realize(state_, SimTK::Stage::Acceleration);
        //4.2 calc reaction forces
        model_.getMultibodySystem().getMatterSubsystem().calcMobilizerReactionForces(state_, reactionForcesInG_);
        mobilizerForces_.clearAllMobilityForces(state_);

    }


    void JointContactFromOpensim::getAllReactionForces(SimTK::Vector_<SimTK::SpatialVec>& reactionForces) const {
        reactionForces = reactionForcesInG_;
    }

}
