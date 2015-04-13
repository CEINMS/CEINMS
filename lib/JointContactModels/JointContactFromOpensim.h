#ifndef Ceinms_JointContactFromOpensim_h
#define Ceinms_JointContactFromOpensim_h

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <OpenSim/OpenSim.h>
#include <OpenSim/Simulation/InverseDynamicsSolver.h>
#include "ExternalForce.h"

namespace CEINMS {

    class JointContactFromOpensim {
    public:
        JointContactFromOpensim(const OpenSim::Model& model);
        void setMuscleForces(const std::vector<double>& forces);
        template<typename T>
        void setQ(const T& q);
        template<typename T>
        void setQd(const T& qd);
        template<typename T>
        void setQdd(const T& qdd);
        void setTime(double time) { state_.updTime() = time; }
        void setEstimatedJointMoments(const std::vector<double>& estimatedJointMoments);
        void calculateReactionForces();
        void calculateIntersegmentalForces();
        void getReactionForce(const std::string& childBodyName, SimTK::Vec3& forceOnChildInChild);
        void getReactionForceInG(const std::string& childBodyName, SimTK::Vec3& forceOnChildInG);


        void getMuscleNamesFromOsimModel(std::vector<std::string>& names) const;
        void getBodyNamesFromOsimModel(std::vector<std::string>& names) const;
        void getCoordinateNamesFromOsimModel(std::vector<std::string>& names) const;
        void getAllReactionForces(SimTK::Vector_<SimTK::Vec3>& reactionForces, SimTK::Vector_<SimTK::Vec3>& reactionTorques) const;
        SimTK::Vector getMobilityForces() const {   return residualMobilityForces_; }

    private:
        OpenSim::Model model_;
        SimTK::State state_;
        std::size_t nCoordinates_, nBodies_, nMuscles_;
        std::list<ExternalForce> externalForces_;
        std::vector<double> estimatedJointMoments_;
        SimTK::Vector_<SimTK::Vec3> rForcesInG_, rTorquesInG_;
        SimTK::Vector residualMobilityForces_;

        std::vector<OpenSim::Actuator_*> muscleActuators_;
        std::vector<OpenSim::CoordinateActuator*> coordinateActuators_;
        std::unique_ptr<OpenSim::InverseDynamicsSolver> idSolver_;
    };

    template<typename T>
    void JointContactFromOpensim::setQ(const T& q) {
        for (size_t i(0); i < nCoordinates_; ++i) {
            state_.updQ()[i] = q[i];
        }
    }

    template<typename T>
    void JointContactFromOpensim::setQd(const T& qd) {
        for (size_t i(0); i < nCoordinates_; ++i) {
            state_.updU()[i] = qd[i];
        }
    }

    template<typename T>
    void JointContactFromOpensim::setQdd(const T& qdd) {
        for (size_t i(0); i < nCoordinates_; ++i) {
            state_.updUDot()[i] = qdd[i];
        }
    }

}

#endif