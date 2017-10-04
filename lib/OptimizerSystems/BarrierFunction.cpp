#include "ceinms/BarrierFunction.h"
#include <limits>
#include <locale>

namespace ceinms {

    BarrierFunction::BarrierFunction(const BarrierSetup& setup) :
        f_(std::numeric_limits<double>::max()),
        setup_(setup),
        barrier_(
            setup.getMinValue(),
            setup.getMaxValue(),
            setup.getWeight(),
            setup.getExponent()) {
    }

    void BarrierFunction::calculate(const std::vector<Result>& results) {
        f_ = 0;
        for (auto& r : results) {
            if (setup_.getCalibrationTargetType() == CalibrationTargetType::Activation)
                f_ += calculatePenalty(r.activations);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::PlanarContactForces)
                f_ += calculatePenalty(r.planarContactForces);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::GenericContactForces)
                f_ += calculatePenalty(r.jointReactionForces);
        }
    }

    double BarrierFunction::getValue() const {
        return f_;
    }

    void BarrierFunction::printCosts() const {
        std::cout << "Fobj = " << getValue() << std::endl;
    }

    double BarrierFunction::calculatePenalty(const DataTable<double>& data) {
        double penalty = 0;
        for (auto& target : setup_.getTargetNames()) {
            int idx = data.getColumnIndex(target);
            if (idx != -1) {
                for (const auto& row : data) {
                    penalty += barrier_.getPenalty(row.at(idx));
                }
            }
        }
        return penalty;
    }

    double BarrierFunction::calculatePenalty(const DataTable<SimTK::Vec3>& data) {
        double penalty = 0;
        for (auto& target : setup_.getTargetNames()) {
            int idx = data.getColumnIndex(target);
            if (idx != -1) {
                for (const auto& row : data) {
                    penalty += barrier_.getPenalty(row.at(idx).norm());
                }
            }
        }
        return penalty;
    }
}
