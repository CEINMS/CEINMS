#include "ceinms/ContactModelKneeBalance.h"
using ceinms::ContactModelKneeBalance;
#include "ceinms/ContactModelKneeBalanceSetup.h"
using ceinms::ContactModelKneeBalanceSetup;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;


int main() {


    ContactModelKneeBalanceSetup setup;
    setup.intercondyleDistance = 0.1;
    setup.condyle1.dof = "medial_condyle_dof";
    setup.condyle1.name = "medial_condyle";
    setup.condyle2.dof = "lateral_condyle_dof";
    setup.condyle2.name = "lateral_condyle";

    double externalMedialTorque = 50;
    double externalLateralTorque = 100;
    double estimatedMedialTorque = 200;
    double estimatedLateralTorque = 300;

    {
        //1. dof names from subject equal condyle dof names
        vector<string> dofNamesFromSubject = { setup.condyle1.dof, setup.condyle2.dof };
        ContactModelKneeBalance contactModelKneeBalance(setup, dofNamesFromSubject);
        contactModelKneeBalance.setTorques({ externalMedialTorque, externalLateralTorque }, { estimatedMedialTorque, estimatedLateralTorque });
        auto forces = contactModelKneeBalance.getContactForces();
        auto names = contactModelKneeBalance.getCondyleNames();
        cout << "Force " << names[0] << " " << forces[0] << endl;
        cout << "Force " << names[1] << " " << forces[1] << endl;
    }

    //2. dof names from subject are more than condyle dof names
    {
        vector<string> dofNamesFromSubject = { "dof1", "dof2", setup.condyle2.dof, "dof3", setup.condyle1.dof, "dof4" };
        vector<double> externalTorques = { 100, 200, externalLateralTorque, 300, externalMedialTorque, 400};
        vector<double> estimatedTorques = { 100, 200, estimatedLateralTorque, 300, estimatedMedialTorque, 400 };

        ContactModelKneeBalance contactModelKneeBalance(setup, dofNamesFromSubject);
        contactModelKneeBalance.setTorques(externalTorques, estimatedTorques);
        auto forces = contactModelKneeBalance.getContactForces();
        auto names = contactModelKneeBalance.getCondyleNames();
        cout << "Force " << names[0] << " " << forces[0] << endl;
        cout << "Force " << names[1] << " " << forces[1] << endl;
    }

    return 0;

}