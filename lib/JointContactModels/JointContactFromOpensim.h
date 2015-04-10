#ifndef Ceinms_JointContactFromOpensim_h
#define Ceinms_JointContactFromOpensim_h

#include <string>
#include <vector>
#include <list>
#include <OpenSim/OpenSim.h>
#include "ExternalForce.h"

namespace CEINMS {

    class JointContactFromOpensim {
    public:
        using VecSPVec = SimTK::Vector_ < SimTK::SpatialVec > ;
      //  JointContactFromOpensim(const std::string& modelFilename);
        JointContactFromOpensim(const OpenSim::Model& model);

        void setMuscleForces(const std::vector<double>& forces);
        void addExternalForce(const ExternalForce& externalForce);

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

        void getReactionForce(const std::string& parentBodyName, const std::string& childBodyName, SimTK::Vec3& forceOnChildInGround);
        void getAllReactionForces(SimTK::Vector_<SimTK::SpatialVec>& reactionForces) const;
        void getMuscleNamesFromOsimModel(std::vector<std::string>& names) const;
        void getBodyNamesFromOsimModel(std::vector<std::string>& names) const;
        void getCoordinateNamesFromOsimModel(std::vector<std::string>& names) const;

        SimTK::Vector getMobilityForces() const {   return residualMobilityForces_; }

        void clearExternalForces() { externalForces_.clear(); }

    private:
    //    void calculateResidualMobilityForces();
        void calculateID(const VecSPVec& appliedBodyForces, const SimTK::Vector& appliedMobilityForces, SimTK::Vector& residualMobilityForces);

        void applyExternalForces(VecSPVec& forces) const;
        void applyGravity(VecSPVec& forces) const;

        OpenSim::Model model_;
        SimTK::State state_;
        std::size_t nCoordinates_, nBodies_, nMuscles_;
        std::list<ExternalForce> externalForces_;
        std::vector<double> estimatedJointMoments_;
        SimTK::Vector_<SimTK::SpatialVec> reactionForcesInG_;
        SimTK::Vector residualMobilityForces_;
        std::vector<OpenSim::Actuator_*> muscleActuators_, coordinateActuators_;

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