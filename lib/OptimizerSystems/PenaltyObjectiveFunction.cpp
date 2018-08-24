#include "ceinms/PenaltyObjectiveFunction.h"
#include <limits>
#include <locale>

namespace ceinms {

    PenaltyObjectiveFunction::PenaltyObjectiveFunction(const PenaltyObjectiveFunctionSetup& setup) :
        f_(std::numeric_limits<double>::max()),
        setup_(setup),
        barrier_(
            setup.getMinValue(),
            setup.getMaxValue(),
            setup.getWeight(),
            setup.getExponent()) {
    }

    void PenaltyObjectiveFunction::calculate(const std::vector<Result>& results) {
        f_ = 0;
        for (auto& r : results) {
            if (setup_.getCalibrationTargetType() == CalibrationTargetType::Activation)
                f_ += calculatePenalty(r.activations);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::ScalarContactForce)
                f_ += calculatePenalty(r.scalarContactForces);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::GenericContactForce)
                f_ += calculatePenalty(r.jointReactionForces);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::NormalisedFibreLength)
                f_ += calculatePenalty(r.normalisedFibreLengths);
            else if (setup_.getCalibrationTargetType() == CalibrationTargetType::NormalisedFibreVelocity)
                f_ += calculatePenalty(r.normalisedFibreVelocities);
        }
        // normalise the weight of the objective function
        // so that the weight of the function doesn't need to change when changing the
        // number of calibrating trials
        f_ /= (results.size()*setup_.getTargetNames().size());
    }

    double PenaltyObjectiveFunction::getValue() const {
        return f_;
    }

    void PenaltyObjectiveFunction::printCosts() const {

        std::cout << setup_;
        std::cout << " - Value: " << getValue() << std::endl;
    }

    double PenaltyObjectiveFunction::calculatePenalty(const DataTable<double>& data) {
        double penalty = 0;
        for (auto& target : setup_.getTargetNames()) {
            double currentPenalty = 0;
            int idx = data.getColumnIndex(target);
            if (idx != -1) {
                for (const auto& row : data) {
                    currentPenalty += barrier_.getPenalty(row.at(idx));
                }
                currentPenalty /= data.getNRows();
            }
            penalty += currentPenalty;
        }
        return penalty;
    }

    double PenaltyObjectiveFunction::calculatePenalty(const DataTable<SimTK::Vec3>& data) {
        double penalty = 0;
        for (auto& target : setup_.getTargetNames()) {
            double currentPenalty = 0;
            int idx = data.getColumnIndex(target);
            if (idx != -1) {
                for (const auto& row : data) {
                    currentPenalty += barrier_.getPenalty(row.at(idx).norm());
                }
                currentPenalty /= data.getNRows();
            }
            penalty += currentPenalty;
        }
        return penalty;
    }
}
