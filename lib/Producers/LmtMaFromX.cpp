//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "LmtMaFromX.h"
#include "DataFromFile.h"
#include "SyncTools.h"

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <cstdlib>

void LmtMaFromX::pushLmtBack(const vector<double>& newLmtToPush)
{ 
  SyncTools::Shared::queueLmtSemEmpty.wait();  //waits if the buffer is full. if so it waits until the consumer removes at least one item from queueLmt
  SyncTools::Shared::queueLmtMutex.lock();
  SyncTools::Shared::queueLmt.push_back(newLmtToPush);
  SyncTools::Shared::queueLmtMutex.unlock();
  SyncTools::Shared::queueLmtSemFull.notify(); //notify that an item has been pushed in the queue
}

void LmtMaFromX::pushMomentArmsBack(const vector<double>& newMomentArmsToPush, unsigned int whichDof)
{ 
  SyncTools::Shared::queueMomentArmsSemEmpty.wait(); //waits if the buffer is full. if so it waits until the consumer removes at least one item from queueMomentArms
  SyncTools::Shared::queueMomentArmsMutex.lock();
  SyncTools::Shared::queueMomentArms.at(whichDof).push_back(newMomentArmsToPush);
  SyncTools::Shared::queueMomentArmsMutex.unlock();
  SyncTools::Shared::queueMomentArmsSemFull.notify(); //notify that an item has been pushed in the queue
}

void LmtMaFromX::updateLmt(const vector<double>& currentLmtData, double currentTime)
{
  vector<double> lmtDataToPush = currentLmtData;
  lmtDataToPush.push_back(currentTime); //appends currentTime at the end
  pushLmtBack(lmtDataToPush);
}

void LmtMaFromX::updateMomentArms(const vector<double>& currentMomentArmsData, double currentTime, unsigned int whichDof)
{
  vector<double> momentArmsDataToPush = currentMomentArmsData;
  momentArmsDataToPush.push_back(currentTime); //appends currentTime at the end
  pushMomentArmsBack(momentArmsDataToPush, whichDof);
}
 
void LmtMaFromX::getDofNames(vector<string>& dofNamesFromModel)
{
  SyncTools::Shared::dofNamesSem.wait();
  dofNamesFromModel = SyncTools::Shared::dofNames; //gets dof names from XML model, passed from global variable dofNames  
  SyncTools::Shared::dofNamesSem.notify();

  setNoDof(dofNamesFromModel.size());
}


void LmtMaFromX::setNoDof(unsigned nDof) {

     SyncTools::Shared::queueMomentArmsMutex.lock();
     SyncTools::Shared::queueMomentArms.resize(nDof);  
     SyncTools::Shared::queueMomentArmsMutex.unlock();

}


void LmtMaFromX::setLmtMusclesNames(const vector<string>& lmtMusclesNames) {

	SyncTools::Shared::musclesNamesMutex.lock();
	if (SyncTools::Shared::musclesNames.empty())         //musclesNames is a global variable, if it isn't empty means that emg set it
		SyncTools::Shared::musclesNames = lmtMusclesNames;
	else if (lmtMusclesNames != SyncTools::Shared::musclesNames) {
		cout << "muscles names are not the same among emg and lmt input data" << endl;
		cout << "lmt names - emg names\n";
		for(unsigned i = 0; i < lmtMusclesNames.size() && i < SyncTools::Shared::musclesNames.size() ; ++i) {
			string lmtName(lmtMusclesNames.at(i)), emgName(SyncTools::Shared::musclesNames.at(i)), separator("  -  ");
			if(lmtName != emgName) separator = "  <>  ";
			cout << lmtName << separator <<  emgName << endl;
		}
		exit(EXIT_FAILURE);      
    }
	SyncTools::Shared::musclesNamesMutex.unlock();
}

void LmtMaFromX::setMomentArmsMusclesNames(const vector< vector<string> >& musclesNamesFromMomentArmsFiles) {

	SyncTools::Shared::musclesNamesOnDofMutex.lock();
	SyncTools::Shared::musclesNamesOnDof = musclesNamesFromMomentArmsFiles;
	SyncTools::Shared::musclesNamesOnDofMutex.unlock();
}


LmtMaFromX::~LmtMaFromX() { }




