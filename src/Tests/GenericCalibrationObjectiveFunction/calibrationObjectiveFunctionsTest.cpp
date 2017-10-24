#include "ceinms/TorqueErrorNormalisedObjectiveFunction.h"
#include "ceinms/MaxPeakObjectiveFunction.h"
#include "ceinms/PenaltyObjectiveFunction.h"
#include "ceinms/ObjectiveFunctionSetup.h"
#include "ceinms/TrialData.h"
#include "ceinms/Result.h"
#include "ceinms/CompositeObjectiveFunction.h"
#include "ceinms/SumGenericContactForces.h"
#include "ceinms/ContactModelKneeBalance.h"
#include "ceinms/ContactModelKneeBalanceSetup.h"
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

std::vector<std::string> getDofNames() {
    return { "dof1", "dof2", "dof3", "dof4", "mdof", "ldof" };
}

std::vector<std::string> getMtuNames() {
    return{ "mtu1", "mtu2", "mtu3" };
}


std::vector<std::string> getCondyleNames() {
    return { "medial_condyle", "lateral_condyle" };
}

bool equals(double a, double b, double tol = 0.001) {
    return (std::fabs(a - b) < tol);
}

/* Trial data is the experimental input data.*/
TrialData getTrialData() {

    vector<double> torqueRow{ 10, 20, 30, 40, 20, 20 };
    DataTable<double> torqueData(3, 6);
    torqueData.setLabels(getDofNames());
    torqueData.setRow(0, torqueRow);
    torqueData.setRow(1, multiply(torqueRow,2));
    torqueData.setRow(2, torqueRow);
    TrialData trial(getDofNames(), getMtuNames(), "test");
    trial.setTorqueData(torqueData);
    return trial;
}

DataTable<double> getContactForces() {

    DataTable<double> contactForces(3,2);
    contactForces.setLabels(getCondyleNames());
    contactForces.setRow(0, { 1000, 2000 });
    contactForces.setRow(1, { 1000, 2000 });
    contactForces.setRow(2, { 1000, 2000 });
    return contactForces;
}

DataTable<SimTK::Vec3> getGenericContactForces() {

    DataTable<SimTK::Vec3> contactForces(3, 2);
    contactForces.setLabels(getCondyleNames());
    contactForces.setRow(0, { {1,2,3}, {4,5,6} });
    contactForces.setRow(1, { { 1,6,3 },{ 1,5,6 } });
    contactForces.setRow(2, { { 1,7,3 },{ 4,5,1 } });
    return contactForces;
}

DataTable<double> getNormalisedFibreLengths() {

    DataTable<double> normFibreLengths(3, 3);
    normFibreLengths.setLabels(getMtuNames());
    normFibreLengths.setRow(0, { 1, 1.3, 0.4 });
    normFibreLengths.setRow(1, { 1, 1.3, 0.4 });
    normFibreLengths.setRow(2, { 1, 1.3, 1 });
    return normFibreLengths;
}

bool testTorqueErrorNormalised() {

    // Generate 3 trials
    TrialData trial1(getTrialData());
    TrialData trial2(getTrialData());
    TrialData trial3 = trial1;
    vector<TrialData> trials{ trial1, trial2, trial3 };

    vector<Result> results;
    for (int i(0); i < 3; ++i) {
        Result result(0, 6, 0, 0);
        result.torques = DataTable<double>::multiplyByScalar(trials[i].getTorqueData(), 1.1);    
        results.push_back(result);
    }
    
    ObjectiveFunctionSetup setup(
        CalibrationObjectiveFunction::TorqueError,
        CalibrationTargetType::Torque,
        getDofNames(), 1, 1);

    bool isValid = true;

    TorqueErrorNormalisedObjectiveFunction fun(setup);
    fun.setTrials(trials);
    fun.calculate(results);
    isValid &= equals(fun.getValue(), 1.08);

    for (auto& d : getDofNames()) {
        setup.setTargetNames({ d });
        fun.setSetup(setup);
        fun.calculate(results);
        isValid &= equals(fun.getValue(), 0.18);
    }

    return isValid;
}

bool testMaxPeak() {

    // Generate 3 trials
    TrialData trial1(getTrialData());
    TrialData trial2(getTrialData());
    TrialData trial3 = trial1;
    vector<TrialData> trials{ trial1, trial2, trial3 };

    vector<Result> results;
    for (int i(0); i < 3; ++i) {
        Result result(0, 6, 0, 0);
        result.torques = DataTable<double>::multiplyByScalar(trials[i].getTorqueData(), 1.1);
        result.planarContactForces = getContactForces();
        result.jointReactionForces = getGenericContactForces();
        results.push_back(result);
    }

    ObjectiveFunctionSetup setup(
        CalibrationObjectiveFunction::MaxPeak,
        CalibrationTargetType::PlanarContactForce,
        getCondyleNames(), 1, 1);

    bool isValid = true;

    MaxPeakObjectiveFunction fun(setup);
    fun.calculate(results);
    isValid &= equals(fun.getValue(), 1500);

    // test first target only
    setup.setTargetNames(getCondyleNames()[0]);
    fun.setSetup(setup);
    fun.calculate(results);
    isValid &= equals(fun.getValue(), 1000);

    // test second target only
    setup.setTargetNames(getCondyleNames()[1]);
    fun.setSetup(setup);
    fun.calculate(results);
    isValid &= equals(fun.getValue(), 2000);

    // test weight 
    setup.setWeight(2);
    setup.setTargetNames(getCondyleNames());
    fun.setSetup(setup);
    fun.calculate(results);
    isValid &= equals(fun.getValue(), 3000);

    //test weight and exponent
    setup.setExponent(2);
    fun.setSetup(setup);
    fun.calculate(results);
    isValid &= equals(fun.getValue(), 2.*(2000*2000+1000*1000)/2);

    setup.setCalibrationTargetType(CalibrationTargetType::GenericContactForce);
    fun.setSetup(setup);
    fun.calculate(results);
    fun.printCosts();

    return isValid;
}


bool testPenaltyObjectiveFunction() {

    // Generate 3 trials
    TrialData trial1(getTrialData());
    TrialData trial2(getTrialData());
    TrialData trial3 = trial1;
    vector<TrialData> trials{ trial1, trial2, trial3 };

    vector<Result> results;
    for (int i(0); i < 3; ++i) {
        Result result(0, 6, 0, 0);
        result.normalisedFibreLengths = getNormalisedFibreLengths();
        results.push_back(result);
    }
    bool isValid = true;
    PenaltyObjectiveFunctionSetup setup(
        CalibrationTargetType::NormalisedFibreLength,
        getMtuNames(), 10000, 2, 0.5, 1.1);

    PenaltyObjectiveFunction fun(setup);
    fun.calculate(results);
    isValid &= equals(fun.getValue(), 155.556);

    return isValid;
}


bool testCompositeFuntion() {

    // Generate 3 trials
    TrialData trial1(getTrialData());
    TrialData trial2(getTrialData());
    TrialData trial3 = trial1;
    vector<TrialData> trials{ trial1, trial2, trial3 };

    vector<Result> results;
    for (int i(0); i < 3; ++i) {
        Result result(0, 6, 0, 0);
        result.normalisedFibreLengths = getNormalisedFibreLengths();
        result.torques = DataTable<double>::multiplyByScalar(trials[i].getTorqueData(), 1.1);
        result.planarContactForces = getContactForces();
        results.push_back(result);
    }
    bool isValid = true;

    std::shared_ptr<ObjectiveFunctionSetup> obf1(
        new ObjectiveFunctionSetup(
            CalibrationObjectiveFunction::MaxPeak,
            CalibrationTargetType::PlanarContactForce,
            getCondyleNames(), 0.001, 1));
  
    std::shared_ptr<ObjectiveFunctionSetup> obf2(
        new ObjectiveFunctionSetup(
            CalibrationObjectiveFunction::TorqueErrorNormalised,
            CalibrationTargetType::Torque,
            getDofNames(), 1, 1));

    std::shared_ptr<PenaltyObjectiveFunctionSetup> obf3(
        new  PenaltyObjectiveFunctionSetup(
            CalibrationTargetType::NormalisedFibreLength,
            getMtuNames(), 100, 2, 0.5, 1.1));

    ObjectiveFunctionSetupList objFunctionsSetup;
    objFunctionsSetup.push_back(obf1);
    objFunctionsSetup.push_back(obf2);
    objFunctionsSetup.push_back(obf3);

    CompositeObjectiveFunction objFunction = CompositeObjectiveFunction::makeComposite(objFunctionsSetup);

    objFunction.setTrials(trials);
    objFunction.calculate(results);
    objFunction.printCosts();
    isValid &= equals(objFunction.getValue(), 1.5+1.08+1.555);
    return isValid;
}


int main() {

    bool isValid = true;
    isValid &= testTorqueErrorNormalised();
    isValid &= testMaxPeak();
    isValid &= testPenaltyObjectiveFunction();
    isValid &= testCompositeFuntion();
    if (isValid)
        return 0;
    return 1;
}
