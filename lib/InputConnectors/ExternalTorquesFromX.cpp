//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "ExternalTorquesFromX.h"
#include "InputQueues.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>


void ExternalTorquesFromX::updateExternalTorques(const vector<double>& currentExternalTorquesData, double currentTime)
{
  QueueData< vector<double> > externalTorquesDataToPush;
  externalTorquesDataToPush.data = currentExternalTorquesData;
  externalTorquesDataToPush.time = currentTime; 
  inputConnectors_.queueExternalTorques.push(externalTorquesDataToPush);
}



ExternalTorquesFromX::~ExternalTorquesFromX() { }




