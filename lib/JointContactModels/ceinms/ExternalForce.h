#ifndef Ceinms_ExternalForce_h
#define Ceinms_ExternalForce_h
#include <ostream>
#include <utility>
#include <SimTK/include/SimTKmath.h>

namespace ceinms {
    class ExternalForce {
    public:
        ExternalForce();
        ExternalForce(unsigned targetBodyIndex);
        void setTargetBody(unsigned targetBodyIndex);
        void setForceVector(const SimTK::Vec3& force, unsigned expressedInBodyIndex = 0);
        void setForceApplicationPoint(const SimTK::Vec3& point, unsigned expressedInBodyIndex = 0);
        void setTorque(const SimTK::Vec3& torque, unsigned expressedInBodyIndex = 0);

        unsigned getTargetBodyIndex() const;

        const SimTK::Vec3& getForce() const;
        void getForce(SimTK::Vec3& force) const;
        unsigned getForceBodyIndex() const;

        const SimTK::Vec3& getApplicationPoint() const;
        void getApplicationPoint(SimTK::Vec3& applicationPoint) const;
        unsigned getApplicationPointBodyIndex() const;

        const SimTK::Vec3& getTorque() const;
        void getTorque(SimTK::Vec3& torque) const;
        unsigned getTorqueBodyIndex() const;

        void print(std::ostream& os) const;
    private:
        unsigned targetBody_;                     //target reference system indicesed to body
        std::pair<unsigned, SimTK::Vec3> point_;  //reference system index and point vector
        std::pair<unsigned, SimTK::Vec3> force_;  //reference system index and force vector
        std::pair<unsigned, SimTK::Vec3> torque_;  //reference system index and force vector

    };
}

#endif