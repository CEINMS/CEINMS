#ifndef Ceinms_JointContactFromOpensim_h
#define Ceinms_JointContactFromOpensim_h

#include <string>
#include <vector>
#include <list>
#include <OpenSim/OpenSim.h>

namespace CEINMS {

    struct ExternalForce {
        unsigned targetBody;                     //target reference system indicesed to body
        std::pair<unsigned, SimTK::Vec3> point;  //reference system index and point vector
        std::pair<unsigned, SimTK::Vec3> force;  //reference system index and force vector

    };

    class JointContactFromOpensim {
    public:
        using VecSPVec = SimTK::Vector_ < SimTK::SpatialVec > ;
      //  JointContactFromOpensim(const std::string& modelFilename);
        JointContactFromOpensim(const OpenSim::Model& model);

        void setMuscleForces(const std::vector<double>& forces);
        void addExternalForce(const ExternalForce& externalForce); 
        void setQ(const std::vector<double>& q);
        void setQd(const std::vector<double>& qd);
        void setQdd(const std::vector<double>& qdd);
        void setEstimatedJointMoments(const std::vector<double>& estimatedJointMoments);
        void calculateReactionForces();
        void calculateIntersegmentalForces();

        void getReactionForce(const std::string& parentBodyName, const std::string& childBodyName, SimTK::Vec3& forceOnChildInGround);
        void getAllReactionForces(SimTK::Vector_<SimTK::SpatialVec>& reactionForces) const;
        void getMuscleNamesFromOsimModel(std::vector<std::string>& names) const;
        void getBodyNamesFromOsimModel(std::vector<std::string>& names) const;
        void getCoordinateNamesFromOsimModel(std::vector<std::string>& names) const;

        void clearExternalForces() { externalForces_.clear(); }

    private:
    //    void calculateResidualMobilityForces();
        void calculateID(const VecSPVec& appliedBodyForces, const SimTK::Vector& appliedMobilityForces, SimTK::Vector& residualMobilityForces);

        void applyExternalForces(VecSPVec& forces) const;
        OpenSim::Model model_;
        SimTK::State state_;
        std::size_t nCoordinates_, nBodies_, nMuscles_;
        std::list<ExternalForce> externalForces_;
        std::vector<double> estimatedJointMoments_;
        SimTK::Force::DiscreteForces mobilizerForces_;
        SimTK::Vector_<SimTK::SpatialVec> reactionForcesInG_;
    };
}

#endif