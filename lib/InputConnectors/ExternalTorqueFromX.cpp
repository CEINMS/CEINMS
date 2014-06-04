//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "ExternalTorqueFromX.h"
#include "InputQueues.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>


void ExternalTorqueFromX::updateExternalTorque(const vector<double>& currentExternalTorqueData, double currentTime, unsigned int whichDof)
{
  vector<double> ExternalTorqueDataToPush = currentExternalTorqueData;
  ExternalTorqueDataToPush.push_back(currentTime);
  CEINMS::InputConnectors::queueExternalTorque.at(whichDof).push(ExternalTorqueDataToPush);
}



ExternalTorqueFromX::~ExternalTorqueFromX() { }




