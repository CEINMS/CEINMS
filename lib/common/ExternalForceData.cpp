#include "ceinms/ExternalForceData.h"
#include <SimTKcommon.h>
using SimTK::Vec3;
#include <utility>
using std::pair;

namespace ceinms {

    ExternalForceData::ExternalForceData() :
        targetBody_(0),
        force_(std::make_pair(0, Vec3(0))),
        point_(std::make_pair(0, Vec3(0))),
        torque_(std::make_pair(0, Vec3(0))) { /*blank*/ }

    ExternalForceData::ExternalForceData(unsigned targetBody) :
        targetBody_(targetBody),
        force_(std::make_pair(0, Vec3(0))),
        point_(std::make_pair(0, Vec3(0))),
        torque_(std::make_pair(0, Vec3(0))) { /*blank*/ }

    void ExternalForceData::setTargetBody(unsigned targetBody) {

        targetBody_ = targetBody;
    }

    void ExternalForceData::setForceVector(const Vec3& forceVector, unsigned expressedInBodyIndex) {

        force_.first = expressedInBodyIndex;
        force_.second = forceVector;
    }

    void ExternalForceData::setForceApplicationPoint(const Vec3& point, unsigned expressedInBodyIndex) {

        point_.first = expressedInBodyIndex;
        point_.second = point;
    }

    void ExternalForceData::setTorque(const Vec3& torque, unsigned expressedInBodyIndex) {

        torque_.first = expressedInBodyIndex;
        torque_.second = torque;
    }

    const Vec3& ExternalForceData::getForce() const {

        return force_.second;
    }

    void ExternalForceData::getForce(Vec3& force) const {

        force = force_.second;
    }

    unsigned ExternalForceData::getForceBodyIndex() const {
        
        return force_.first;
    }

    const Vec3& ExternalForceData::getApplicationPoint() const {
    
        return point_.second;
    }

    void ExternalForceData::getApplicationPoint(Vec3& applicationPoint) const {

        applicationPoint = point_.second;
    }
    
    unsigned ExternalForceData::getApplicationPointBodyIndex() const {
        
        return point_.first;
    }

    const Vec3& ExternalForceData::getTorque() const {

        return torque_.second;
    }

    void ExternalForceData::getTorque(Vec3& torque) const {

        torque = torque_.second;
    }

    unsigned ExternalForceData::getTorqueBodyIndex() const {

        return torque_.first;
    }


    unsigned ExternalForceData::getTargetBodyIndex() const {

        return targetBody_;
    }

    void ExternalForceData::print(std::ostream& os) const {

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