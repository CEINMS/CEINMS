#include "ceinms/ComposedObjectiveFunction.h"

namespace ceinms {

    ComposedObjectiveFunction::ComposedObjectiveFunction() :
        f_(std::numeric_limits<double>::max()) {}

    ComposedObjectiveFunction::ComposedObjectiveFunction(const std::vector<std::shared_ptr<ObjectiveFunctionBase>>& objectiveFunctions) :
        ComposedObjectiveFunction() {

        objectiveFunctions_ = objectiveFunctions;
    }

    void ComposedObjectiveFunction::appendObjectiveFunction(std::shared_ptr<ObjectiveFunctionBase> objectiveFunction) {
        
        objectiveFunctions_.emplace_back(objectiveFunction);
        objectiveFunctions_.back()->setDofsToCalibrate(dofsToCalibrate_);
        objectiveFunctions_.back()->setTrials(trials_);
    }
    
    void ComposedObjectiveFunction::setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate) {

        dofsToCalibrate_ = dofsToCalibrate;
        for (auto& it : objectiveFunctions_)
            it->setDofsToCalibrate(dofsToCalibrate);
    }

    void ComposedObjectiveFunction::setTrials(const std::vector<TrialData>& trials) {

        trials_ = trials;
        for (auto& it : objectiveFunctions_)
            it->setTrials(trials_);
    }

    void ComposedObjectiveFunction::calculate(const std::vector<Result>& results) {
        f_ = 0;
        for (auto& it : objectiveFunctions_) {
            it->calculate(results);
            f_ += it->getValue();
        }
    }


    double ComposedObjectiveFunction::getValue() const {

        return f_;
    }

    void ComposedObjectiveFunction::printCosts() const {

        for (auto& it : objectiveFunctions_)
            it->printCosts();

    }

    bool ComposedObjectiveFunction::calculateOpenSimData() const {

        bool ans(false);
        for (auto& obj : objectiveFunctions_)
            ans |= obj->calculateOpenSimData();
        return ans;
    }
}