//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "EMGFromX.h"
#include "InputQueues.h"

#include <string>
using std::string;
#include <vector>
using std::vector;

#include <cstdlib>


void EMGFromX::updateEmg(const vector<double>& currentEmgData, double currentTime)
{
  vector<double> emgDataToPush = currentEmgData;
  emgDataToPush.push_back(currentTime); 
  CEINMS::InputConnectors::queueEmg.push(emgDataToPush);
}


EMGFromX::~EMGFromX() { }



