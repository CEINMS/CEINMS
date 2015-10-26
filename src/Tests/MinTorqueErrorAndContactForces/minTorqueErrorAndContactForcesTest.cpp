#include "ceinms/MinTorqueErrorAndContactForces.h"
#include "ceinms/MinTorqueError.h"
#include "ceinms/SumKneeContactForces.h"
#include "ceinms/ObjectiveFunctionSetup.h"
#include "ceinms/TrialData.h"
#include "ceinms/Result.h"
#include "ceinms/Barrier.h"

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

int main() {

    ObjectiveFunctionSetup objectiveFunctionSetup;
    objectiveFunctionSetup.exponent = 2;
    objectiveFunctionSetup.weight = 1;

    ContactModelKneeBalanceSetup contactModelKneeBalanceSetup;
    contactModelKneeBalanceSetup.intercondyleDistance = 0.1;
    contactModelKneeBalanceSetup.condyle1.dof = "medial_condyle_dof";
    contactModelKneeBalanceSetup.condyle1.name = "medial_condyle";
    contactModelKneeBalanceSetup.condyle2.dof = "lateral_condyle_dof";
    contactModelKneeBalanceSetup.condyle2.name = "lateral_condyle";

    SumKneeContactForces sumKneeContacts(objectiveFunctionSetup);
    sumKneeContacts.setContactModelSetup(contactModelKneeBalanceSetup);
    MinTorqueError minTorqueError;
    MinTorqueErrorAndContactForces minTorqueErrorAndContactForces(minTorqueError, sumKneeContacts);

    vector<string> dofNamesFromSubject = { "dof1", "dof2", "dof3", "dof4", contactModelKneeBalanceSetup.condyle1.dof, contactModelKneeBalanceSetup.condyle2.dof };

    vector<double> torqueRow{ 10, 10, 10, 10, 20, 20 };
    TrialData trial1;
    trial1.dofNames = dofNamesFromSubject;
    trial1.noDoF = 6;
    trial1.torqueData.pushRow(0, torqueRow);
    trial1.torqueData.pushRow(1, torqueRow);

    Result result(0, 6, 0, 0);
    result.torques.pushRow(0, multiply(torqueRow, 1.1));
    result.torques.pushRow(1, multiply(torqueRow, 1.1));

    minTorqueErrorAndContactForces.setDofsToCalibrate({ "dof1", "dof2", "dof3", "dof4" });
    minTorqueErrorAndContactForces.setTrials({ trial1 });
    minTorqueErrorAndContactForces.calculate({ result });
    cout << "f_ " << minTorqueErrorAndContactForces.getValue() << endl;

    return 0;

}