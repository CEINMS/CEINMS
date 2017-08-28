#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include <functional>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <memory>

using namespace ceinms;
using ExternalDataMap = std::map<std::string, std::vector<double>>;
using ExternalDataMapPtr = std::shared_ptr<ExternalDataMap>;

struct ObjectiveFunctionComponent {

    std::string type_; //"torque", "force", "excitation" 
    double weight_;
    double exponent_;
    std::string action_; //"none", "difference"
    std::vector<std::string> sources_; //names.. such as 
};


template<typename NMSmodelT>
std::function<double(NMSmodelT&, ExternalDataMapPtr)> getFunction(
    ObjectiveFunctionComponent& component, NMSmodelT& model, ExternalDataMapPtr externalDataMap = nullptr) {

    auto cumsumFun([](const std::vector<double>& vec) {
        return std::accumulate(vec.cbegin(), vec.cend(), 0.);
    });

    auto diffFun([](const std::vector<double>& vec1,
        const std::vector<double>& vec2) {
        std::vector<double> results;
        std::transform(vec1.cbegin(), vec1.cend(), vec2.cbegin(), std::back_inserter(results),
            [](double first, double second) {
            return first - second;
        });
        return results;
    });

    auto powFunc([component](const std::vector<double>& vec) {
        std::vector<double> results;
        std::transform(vec.cbegin(), vec.cend(), std::back_inserter(results),
            [component](double value) {
            return std::pow(value, component.exponent_);
        });
        return results;
    });

    auto getData([component](const NMSmodelT& model) {
        std::vector<double> data;
        if (component.type_ == "emg")
            model.getEmgs(data);
        else if (component.type_ == "force")
            model.getMuscleForces(data);
        else if (component.type_ == "torque")
            model.getTorques(data);
        return data;
    });

    auto weightSum([component, cumsumFun](const std::vector<double>& vec) {
        return component.weight_*cumsumFun(vec);
    });
    
    auto compositeFunction([=](const NMSmodelT& currentModel, ExternalDataMapPtr externalDataMap) {
        std::vector<double> modelData(getData(currentModel));
        std::vector<double> data;
        if (component.action_ != "difference" || externalDataMap == nullptr) {
            data = modelData;
        }
        else {
            auto it = externalDataMap->find(component.type_);
            if (it != externalDataMap->cend())
                data = diffFun(modelData, it->second);
        }
        return weightSum(powFunc(data)); });

     return compositeFunction;
}

class GenericHybridFunction {
public:
    GenericHybridFunction(std::vector<ObjectiveFunctionComponent>& components);

    template<typename NMSmodelT>
    double getValue(const NMSmodelT, ExternalDataMapPtr externalData) {
        for(auto& fun)

    }
private:
    std::vector<ObjectiveFunctionComponent> components_;
};


int main() {


    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> model;
    MTU<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> mtu1("mtu1");
    MTU<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> mtu2("mtu2");
    MTU<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> mtu3("mtu3");
    model.addMuscle(mtu1);
    model.addMuscle(mtu2);
    model.addMuscle(mtu3);

    ObjectiveFunctionComponent component;
    component.weight_ = 1;
    component.exponent_ = 2;
    component.type_ = "emg";

    auto func = getFunction(component, model);
    model.setEmgs({ 0.1, 0.2, 0.3 });
    std::cout << func(model, nullptr) << std::endl;
    model.setEmgs({ 0.1, 0.1, 0.1 });
    std::cout << func(model, nullptr) << std::endl;

    ExternalDataMapPtr externalData = std::make_shared<ExternalDataMap>();
    externalData->insert(std::make_pair("emg", std::vector<double>{ 1,1,1 }));
    component.action_ = "difference";
    auto func2 = getFunction(component, model, externalData);
    std::cout << func2(model, externalData) << std::endl;
    std::cout << func(model, nullptr) << std::endl;

    return 0;
}