//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "EMGFromX.h"
#include "InputConnectors.h"

#include <string>
using std::string;
#include <vector>
using std::vector;
#include "QueueData.h"
#include <cstdlib>


void EMGFromX::updateEmg(const vector<double>& currentEmgData, double currentTime)
{
  QueueData< vector<double> > emgDataToPush; 
  emgDataToPush.data = currentEmgData;
  emgDataToPush.time = currentTime; 
  inputConnectors_.queueEmg.push(emgDataToPush);
}


EMGFromX::~EMGFromX() { }



