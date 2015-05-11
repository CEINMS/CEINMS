#include "ceinms/ExternalForce.h"
#include <SimTK/include/SimTKmath.h>
using SimTK::Vec3;
#include <utility>
using std::pair;

namespace ceinms {

    ExternalForce::ExternalForce() :
        targetBody_(0),
        force_(std::make_pair(0, Vec3(0))),
        point_(std::make_pair(0, Vec3(0))),
        torque_(std::make_pair(0, Vec3(0))) { /*blank*/ }

    ExternalForce::ExternalForce(unsigned targetBody) :
        targetBody_(targetBody),
        force_(std::make_pair(0, Vec3(0))),
        point_(std::make_pair(0, Vec3(0))),
        torque_(std::make_pair(0, Vec3(0))) { /*blank*/ }

    void ExternalForce::setTargetBody(unsigned targetBody) {

        targetBody_ = targetBody;
    }

    void ExternalForce::setForceVector(const Vec3& forceVector, unsigned expressedInBodyIndex) {

        force_.first = expressedInBodyIndex;
        force_.second = forceVector;
    }

    void ExternalForce::setForceApplicationPoint(const Vec3& point, unsigned expressedInBodyIndex) {

        point_.first = expressedInBodyIndex;
        point_.second = point;
    }

    void ExternalForce::setTorque(const Vec3& torque, unsigned expressedInBodyIndex) {

        torque_.first = expressedInBodyIndex;
        torque_.second = torque;
    }

    const Vec3& ExternalForce::getForce() const {

        return force_.second;
    }

    void ExternalForce::getForce(Vec3& force) const {

        force = force_.second;
    }

    unsigned ExternalForce::getForceBodyIndex() const {
        
        return force_.first;
    }

    const Vec3& ExternalForce::getApplicationPoint() const {
    
        return point_.second;
    }

    void ExternalForce::getApplicationPoint(Vec3& applicationPoint) const {

        applicationPoint = point_.second;
    }
    
    unsigned ExternalForce::getApplicationPointBodyIndex() const {
        
        return point_.first;
    }

    const Vec3& ExternalForce::getTorque() const {

        return torque_.second;
    }

    void ExternalForce::getTorque(Vec3& torque) const {

        torque = torque_.second;
    }

    unsigned ExternalForce::getTorqueBodyIndex() const {

        return torque_.first;
    }


    unsigned ExternalForce::getTargetBodyIndex() const {

        return targetBody_;
    }

    void ExternalForce::print(std::ostream& os) const {

        os << "Target body index " << getTargetBodyIndex() << std::endl;
        os << "Force" << std::endl;
        os << "--Reference System Index " << getForceBodyIndex() << std::endl;
        os << "--Value                  " << getForce() << std::endl;
        os << "Force Application Point" << std::endl;
        os << "--Reference System Index " << getApplicationPointBodyIndex() << std::endl;
        os << "--Value                  " << getApplicationPoint() << std::endl;
        os << "Torque" << std::endl;
        os << "--Reference System Index " << getTorqueBodyIndex() << std::endl;
        os << "--Value                  " << getTorque() << std::endl;
    }

}