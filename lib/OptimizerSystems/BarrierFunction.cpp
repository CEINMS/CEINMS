#include "ceinms/BarrierFunction.h"

namespace ceinms {

    BarrierFunction::BarrierFunction() : f_(std::numeric_limits<double>::max()) {}

    void BarrierFunction::initialize(const BarrierListSetup& setup, const std::vector<std::string>& namesFromInput) {

        for (auto it : setup.barrierSetup) {
            barriers_.push_back(Barrier(it.xL, it.xR, it.weight, it.exponent));
            barrierTargetNames_.push_back(it.targetName);
        }
        barrierMapper_.setNames(namesFromInput, barrierTargetNames_);
    }

    double BarrierFunction::getValue(const std::vector<double>& x) const {

        double f = 0;
        auto orderedX = barrierMapper_.map(x);
        for (unsigned i(0); i < barriers_.size(); ++i)
            f += barriers_.at(i).getPenalty(orderedX.at(i));
        return f;
    }
}