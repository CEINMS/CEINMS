//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "LmtMaFromX.h"
#include "InputQueues.h"

using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>


void LmtMaFromX::updateLmt(const vector<double>& currentLmtData, double currentTime)
{
  QueueData< vector<double> > lmtDataToPush;
  lmtDataToPush.data = currentLmtData; 
  lmtDataToPush.time = currentTime; 
  CEINMS::InputConnectors::queueLmt.push(lmtDataToPush);
}

void LmtMaFromX::updateMomentArms(const vector<double>& currentMomentArmsData, double currentTime, unsigned int whichDof)
{
  QueueData< vector<double> > momentArmsDataToPush; 
  momentArmsDataToPush.data = currentMomentArmsData; 
  momentArmsDataToPush.time = currentTime; 
  (*CEINMS::InputConnectors::queueMomentArms.at(whichDof)).push(momentArmsDataToPush);
}

LmtMaFromX::~LmtMaFromX() { }




