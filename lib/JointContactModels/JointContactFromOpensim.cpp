#include "JointContactFromOpensim.h"
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>

#include "ArrayConverter.h"
#include <OpenSim/Simulation/InverseDynamicsSolver.h>
namespace CEINMS {
 
    JointContactFromOpensim::JointContactFromOpensim(const OpenSim::Model& model) :
        model_(model),
        state_(model_.initSystem()),
        nCoordinates_(model_.getNumCoordinates()),
        nBodies_(model_.getNumBodies()),
        nMuscles_(model_.getMuscles().getSize()),
        mobilizerForces_(),
//        mobilizerForces_(model_.updForceSubsystem(), model_.getMatterSubsystem()),
        reactionForcesInG_(nCoordinates_, SimTK::SpatialVec(SimTK::Vec3(.0), SimTK::Vec3(.0))){
       
            
        std::cout << model_.updForceSubsystem().adoptForce(SimTK::Force::DiscreteForces());
           // adoptForce(mobilizerForces_);

       //blank
    }


    void JointContactFromOpensim::addExternalForce(const ExternalForce& externalForce) {
        externalForces_.push_front(externalForce);
   //     externalForces_.front().print(std::cout);
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
            SimTK::Vec3 pAppliedBodyInB(0), fAppliedBodyInG(0);
            //transform position in body and force in ground
      //      std::cout << model_.getBodySet().get(f.getApplicationPointBodyIndex()).getName() << std::endl;
      //      std::cout << model_.getBodySet().get(f.getTargetBodyIndex()).getName() << std::endl;

            model_.getSimbodyEngine().transformPosition(state_,
                                                        model_.getBodySet().get(f.getApplicationPointBodyIndex()),
                                                        f.getApplicationPoint(),
                                                        model_.getBodySet().get(f.getTargetBodyIndex()),
                                                        pAppliedBodyInB);
 //           std::cout << "f.getApplicationPoint() " << f.getApplicationPoint() << std::endl;

  //          std::cout << "pallpliedBodyInB " << pAppliedBodyInB << std::endl;
            model_.getSimbodyEngine().transform(state_,
                                                model_.getBodySet().get(f.getForceBodyIndex()),
                                                f.getForce(),
                                                model_.getGroundBody(),
                                                fAppliedBodyInG);
 //           std::cout << "f.getForce() " << f.getForce() << std::endl;

 //           std::cout << "fAppliedBodyInG " << fAppliedBodyInG << std::endl;

            SimTK::MobilizedBodyIndex index(f.getTargetBodyIndex());
            model_.getMatterSubsystem().getMobilizedBody(index).applyForceToBodyPoint(state_, 
                                                                                      pAppliedBodyInB, 
                                                                                      fAppliedBodyInG, 
                                                                                      forces);

            //torque should be transformed in ground first..
            model_.getMatterSubsystem().getMobilizedBody(index).applyBodyTorque(state_,
                f.getTorque(),
                forces);

        }
    }

    void JointContactFromOpensim::applyGravity(SimTK::Vector_<SimTK::SpatialVec>& forces) const {

        auto gravity(model_.getGravity());
        const OpenSim::BodySet& bodies(model_.getBodySet());
        for (unsigned i(0); i < bodies.getSize(); ++i) {
            SimTK::Vec3 centreOfMass;
            bodies.get(i).getMassCenter(centreOfMass);
            SimTK::MobilizedBodyIndex index(i);
            model_.getMatterSubsystem().getMobilizedBody(index).applyForceToBodyPoint(state_,
                centreOfMass,
                gravity,
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

        model_.getMultibodySystem().realize(state_, SimTK::Stage::Dynamics);
        model_.getMatterSubsystem().calcResidualForceIgnoringConstraints(state_, appliedMobilityForces, appliedBodyForces, state_.updQDotDot(), residualMobilityForces);

    }

    void JointContactFromOpensim::calculateIntersegmentalForces() {

    //  todo: improve muscle disabling.. at the moment it assumes just muscles are in the force set
        for (size_t i(0); i < nMuscles_; ++i)
            model_.updForceSet().get(i).setDisabled(state_, true);

        //realize the system, so all the forces as gravitational, inertial, and external forces are applied
        model_.getMultibodySystem().realize(state_, SimTK::Stage::Dynamics);
        SimTK::Vector_<SimTK::SpatialVec> appliedBodyForces(model_.getMultibodySystem().getRigidBodyForces(state_, SimTK::Stage::Dynamics));
        const SimTK::Vector &appliedMobilityForces = model_.getMultibodySystem().getMobilityForces(state_, SimTK::Stage::Dynamics);
        applyExternalForces(appliedBodyForces);
  //      std::cout << appliedBodyForces.size() << appliedBodyForces << std::endl;
     //   SimTK::Vector residualMobilityForces(nCoordinates_);

        //todo: make the solves a member variable.. osim solver works, while my function call doesn't .. who knows..
        OpenSim::InverseDynamicsSolver idSolver(model_);
        residualMobilityForces_ = idSolver.solve(state_, state_.getUDot(), appliedMobilityForces, appliedBodyForces);
//        calculateID(appliedBodyForces, appliedMobilityForces, residualMobilityForces);

        //4 apply id results to model 
        //4.1 apply generalised forces to the system
       
     //   mobilizerF.setAllMobilityForces(state_, residualMobilityForces_);
  //      mobilizerForces_.setAllMobilityForces(state_, residualMobilityForces_);
 

        //       model_.getMultibodySystem().realize(state_, SimTK::Stage::Acceleration);
        //4.2 calc reaction forces
  //      model_.getMultibodySystem().getMatterSubsystem().calcMobilizerReactionForces(state_, reactionForcesInG_);
//        mobilizerForces_.clearAllMobilityForces(state_);

    }


    void JointContactFromOpensim::getAllReactionForces(SimTK::Vector_<SimTK::SpatialVec>& reactionForces) const {
        reactionForces = reactionForcesInG_;
    }

    

}
