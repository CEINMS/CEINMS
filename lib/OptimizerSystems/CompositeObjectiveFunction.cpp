#include "ceinms/CompositeObjectiveFunction.h"
#include "ceinms/TorqueErrorNormalisedObjectiveFunction.h"
#include "ceinms/MaxPeakObjectiveFunction.h"
#include "ceinms/PenaltyObjectiveFunction.h"

namespace ceinms {

    CompositeObjectiveFunction::CompositeObjectiveFunction() :
        f_(std::numeric_limits<double>::max()) {}

    CompositeObjectiveFunction::CompositeObjectiveFunction(
        const std::vector<std::shared_ptr<ObjectiveFunctionBase>>& objectiveFunctions) :
        CompositeObjectiveFunction() {

        objectiveFunctions_ = objectiveFunctions;
    }

    void CompositeObjectiveFunction::appendObjectiveFunction(std::shared_ptr<ObjectiveFunctionBase> objectiveFunction) {
        
        objectiveFunctions_.emplace_back(objectiveFunction);
        objectiveFunctions_.back()->setTrials(trials_);
    }
    
    void CompositeObjectiveFunction::setTrials(const std::vector<TrialData>& trials) {
        
        trials_ = trials;
        for (auto& it : objectiveFunctions_)
            it->setTrials(trials);
    }

    void CompositeObjectiveFunction::calculate(const std::vector<Result>& results) {
        f_ = 0;
        for (auto& it : objectiveFunctions_) {
            it->calculate(results);
            f_ += it->getValue();
        }
    }

    double CompositeObjectiveFunction::getValue() const {
        return f_;
    }

    void CompositeObjectiveFunction::printCosts() const {

        std::cout << "------------------------------------------------------------------------------\n";
        for (auto& it : objectiveFunctions_)
            it->printCosts();
        std::cout << "------------------------------------------------------------------------------\n";
        std::cout << "   Fobj = " << getValue() << std::endl;
        std::cout << "------------------------------------------------------------------------------\n";

    }

    bool CompositeObjectiveFunction::calculateOpenSimData() const {

        bool ans(false);
        for (auto& obj : objectiveFunctions_)
            ans |= obj->calculateOpenSimData();
        return ans;
    }

    CompositeObjectiveFunction CompositeObjectiveFunction::makeComposite(
        ObjectiveFunctionSetupList objectiveFunctions) {

        CompositeObjectiveFunction objFunction;
        for (auto e : objectiveFunctions) {
            if (e->getCalibrationObjectiveFunction()
                == CalibrationObjectiveFunction::TorqueErrorNormalised) {
                std::shared_ptr<TorqueErrorNormalisedObjectiveFunction> obj(
                    new TorqueErrorNormalisedObjectiveFunction(*e));
                objFunction.appendObjectiveFunction(obj);
            }
            else if (e->getCalibrationObjectiveFunction()
                == CalibrationObjectiveFunction::MaxPeak) {
                std::shared_ptr<MaxPeakObjectiveFunction> obj(new MaxPeakObjectiveFunction(*e));
                objFunction.appendObjectiveFunction(obj);
            }
            //      else if (e->getCalibrationObjectiveFunction()
            //          == CalibrationObjectiveFunction::PeakGenericContactForces) {
            //          std::shared_ptr<SumGenericContactForces> obj(new SumGenericContactForces(*e));
            //          objFunction.appendObjectiveFunction(obj);
            //      }
            else if (e->getCalibrationObjectiveFunction()
                == CalibrationObjectiveFunction::Penalty) {
                std::shared_ptr<PenaltyObjectiveFunction> obj(
                    new PenaltyObjectiveFunction(
                    *std::dynamic_pointer_cast<PenaltyObjectiveFunctionSetup>(e)));
                objFunction.appendObjectiveFunction(obj);
            }
        }
        return objFunction;
    }

}