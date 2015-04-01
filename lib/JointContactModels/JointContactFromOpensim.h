#ifndef Ceinms_JointContactFromOpensim_h
#define Ceinms_JointContactFromOpensim_h

#include <string>
#include <vector>
#include <list>
#include <OpenSim/OpenSim.h>

namespace CEINMS {

    struct ExternalForce {
        std::pair<unsigned, SimTK::Vec3> point;  //body of application index and position vector
        std::pair<unsigned, SimTK::Vec3> force;  //body of application index and force vector

    };

    class JointContactFromOpensim {
    public:
      //  JointContactFromOpensim(const std::string& modelFilename);
        JointContactFromOpensim(const OpenSim::Model& model);

        void setMuscleForces(const std::vector<double>& forces);
        void addExternalForceToMobilizedBodyPoint(const ExternalForce& externalForce); 
        void setQ(const std::vector<double>& q);
        void setQd(const std::vector<double>& qd);
        void setQdd(const std::vector<double>& qdd);
        void setEstimatedJointMoments(const std::vector<double>& estimatedJointMoments);
        void calculateReactionForces();
        void calculateIntersegmentalForces();

        void getReactionForce(const std::string& parentBodyName, const std::string& childBodyName, SimTK::Vec3& forceOnChildInGround);
        void getMuscleNamesFromOsimModel(std::vector<std::string>& names) const;
        void getBodyNamesFromOsimModel(std::vector<std::string>& names) const;
        void getCoordinateNamesFromOsimModel(std::vector<std::string>& names) const;


    private:
    //    void calculateResidualMobilityForces();
        void applyExternalForces(SimTK::Vector_<SimTK::SpatialVec>& forces) const;
        OpenSim::Model model_;
        SimTK::State state_;
        std::size_t nCoordinates_, nBodies_, nMuscles_;
        std::list<ExternalForce> externalForces_;
        std::vector<double> estimatedJointMoments_;

    };
}

#endif