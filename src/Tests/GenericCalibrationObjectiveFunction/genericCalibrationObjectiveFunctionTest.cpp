#include "ceinms/MinTorqueErrorAndContactForces.h"
#include "ceinms/MinTorqueError.h"
#include "ceinms/SumKneeContactForces.h"
#include "ceinms/ObjectiveFunctionSetup.h"
#include "ceinms/TrialData.h"
#include "ceinms/Result.h"
#include "ceinms/Barrier.h"
#include "ceinms/ComposedObjectiveFunction.h"
#include "ceinms/SumGenericContactForces.h"

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

using namespace ceinms;

vector<double> multiply(vector<double> values, double c) {
    for (auto& v : values) {
        v *= c;
    }
    return values;
}

ComposedObjectiveFunction makeComposedObjFunction(ObjectiveFunctionListSetup objFunListSetup) {

    ComposedObjectiveFunction objFunction;
    for (auto e : objFunListSetup.objectiveFunctionSetup) {
        if (e->getCalibrationObjectiveFunction() 
            == CalibrationObjectiveFunction::TorqueError) {
            std::shared_ptr<MinTorqueError> obj(new MinTorqueError(*e));
            objFunction.appendObjectiveFunction(obj);
        }
        else if (e->getCalibrationObjectiveFunction()
            == CalibrationObjectiveFunction::Peak) {
            std::shared_ptr<SumKneeContactForces> obj(new SumKneeContactForces(*e));
            objFunction.appendObjectiveFunction(obj);
        }
  //      else if (e->getCalibrationObjectiveFunction()
  //          == CalibrationObjectiveFunction::PeakGenericContactForces) {
  //          std::shared_ptr<SumGenericContactForces> obj(new SumGenericContactForces(*e));
  //          objFunction.appendObjectiveFunction(obj);
  //      }
        else if (e->getCalibrationObjectiveFunction()
            == CalibrationObjectiveFunction::Penalty) {
            std::shared_ptr<BarrierFunction> obj(new BarrierFunction(
                *std::dynamic_pointer_cast<BarrierSetup>(e)));
            objFunction.appendObjectiveFunction(obj);
        }
    }
    return objFunction;
}


int main() {

    std::shared_ptr<ObjectiveFunctionSetup> objectiveFunction1(
        new ObjectiveFunctionSetup(
        CalibrationObjectiveFunction::Peak, 
        CalibrationTargetType::PlanarContactForces,
        { "medial_condyle", "lateral_condyle" }, 0.000001, 2));
    std::shared_ptr<ObjectiveFunctionSetup> objectiveFunction2(
        new ObjectiveFunctionSetup(
        CalibrationObjectiveFunction::TorqueError,
        CalibrationTargetType::Torque,
        { "dof1", "dof2", "dof3", "dof4" }, 1, 1));

    std::shared_ptr<BarrierSetup> barrierSetup1(
        new BarrierSetup(
        CalibrationTargetType::PlanarContactForces, 
        { "medial_condyle", "lateral_condyle" }, 1,2,0, 1990));
  
    ObjectiveFunctionListSetup objFunctionsSetup;
    objFunctionsSetup.objectiveFunctionSetup.push_back(objectiveFunction1);
    objFunctionsSetup.objectiveFunctionSetup.push_back(objectiveFunction2);
    objFunctionsSetup.objectiveFunctionSetup.push_back(barrierSetup1);

    ComposedObjectiveFunction objFunction = makeComposedObjFunction(objFunctionsSetup);

    ContactModelKneeBalanceSetup contactModelKneeBalanceSetup;
    contactModelKneeBalanceSetup.intercondyleDistance = 0.1;
    contactModelKneeBalanceSetup.condyle1.dof = "medial_condyle_dof";
    contactModelKneeBalanceSetup.condyle1.name = "medial_condyle";
    contactModelKneeBalanceSetup.condyle2.dof = "lateral_condyle_dof";
    contactModelKneeBalanceSetup.condyle2.name = "lateral_condyle";

    vector<string> dofNamesFromSubject = { "dof1", "dof2", "dof3", "dof4", contactModelKneeBalanceSetup.condyle1.dof, contactModelKneeBalanceSetup.condyle2.dof };

    vector<double> torqueRow{ 10, 10, 10, 10, 20, 20 };
    vector<double> contactForcesRow{ 1000, 2000};

    DataTable<double> torqueData(2, 6);
    torqueData.setLabels(dofNamesFromSubject);
    torqueData.setRow(0, torqueRow);
    torqueData.setRow(1, torqueRow);
    DataTable<double> contactForces(2, 2);
    contactForces.setLabels({ contactModelKneeBalanceSetup.condyle1.name, contactModelKneeBalanceSetup.condyle2.name });
    contactForces.setRow(0, contactForcesRow);
    contactForces.setRow(1, contactForcesRow);
    TrialData trial1(dofNamesFromSubject, {}, "test");
    trial1.setTorqueData(torqueData);
    Result result(0, 6, 0, 0);
    result.torques.pushRow(0, multiply(torqueRow, 1.1));
    result.torques.pushRow(1, multiply(torqueRow, 1.1));
    result.planarContactForces = contactForces;
    objFunction.setTrials({ trial1 });
    objFunction.calculate({ result });
    cout << "f_ " << objFunction.getValue() << endl; 

    return 0;

}