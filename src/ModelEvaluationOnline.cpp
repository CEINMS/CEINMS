//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//



#include "SimpleFileLogger.h"
#include "StorageLogger.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
#include <boost/concept_check.hpp>
using std::string;

#include "TimeCompare.h"
#include "ModelEvaluationOnline.h"
#include "ModelEvaluationBase.h"
#include "InputConnectors.h"
#include "OutputConnectors.h"
using namespace CEINMS;
#define LOG_FILES
#define LOG


template <typename NMSmodelT, typename Logger>
ModelEvaluationOnline<NMSmodelT, Logger>::ModelEvaluationOnline(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, NMSmodelT& subject, const vector<string>& valuesToLog) //const std::string& outputDir )
:ModelEvaluationBase<Logger>::ModelEvaluationBase(inputConnectors, outputConnectors, valuesToLog), subject_(subject)
{
  subject_.getDoFNames(dofNames_);
  noDof_ = dofNames_.size(); 
  globalEmDelay_ = subject.getGlobalEmDelay(); 
}

template <typename NMSmodelT, typename Logger>
void ModelEvaluationOnline<NMSmodelT, Logger>::operator()() {

    ModelEvaluationBase<Logger>::subscribeToInputConnectors();
  
    #ifdef LOG
    cout << "starting consume" << endl;
    #endif
  
    double externalTorqueTime = std::numeric_limits<double>::lowest();
    double emgTime = std::numeric_limits<double>::lowest();
    bool runCondition = true; 
    bool firstLmtArrived(false);
    do {  // while(runCondition)
     
        // 1. read lmt Data
        CEINMS::InputConnectors::FrameType lmtFrameFromQueue;
        ModelEvaluationBase<Logger>::getLmtFromInputQueue(lmtFrameFromQueue);
        double lmtMaTime = lmtFrameFromQueue.time;

        // update runCondition
        runCondition = runCondition && !lmtFrameFromQueue.data.empty();

        // 2. read moment arms data
        vector< CEINMS::InputConnectors::FrameType > momentArmsFrameFromQueue;
        momentArmsFrameFromQueue.resize(noDof_);
        for(unsigned int i = 0; i < noDof_; ++i) {
          ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue((momentArmsFrameFromQueue.at(i)), i);
          runCondition = runCondition && !momentArmsFrameFromQueue.at(i).data.empty();
        }

        // 3. read external Torque 
        CEINMS::InputConnectors::FrameType externalTorquesFrameFromQueue;  
        if (ModelEvaluationBase<Logger>::externalTorquesAvailable()) {
            while (TimeCompare::less(externalTorqueTime, lmtMaTime)) {
                ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue(externalTorquesFrameFromQueue);
                externalTorqueTime = externalTorquesFrameFromQueue.time;
            }
        }

        // 4. read emgs
        CEINMS::InputConnectors::FrameType emgFrameFromQueue;    
        while (TimeCompare::less(emgTime, lmtMaTime) && runCondition) {
            ModelEvaluationBase<Logger>::getEmgFromInputQueue(emgFrameFromQueue);
            emgTime = emgFrameFromQueue.time + globalEmDelay_;
            runCondition = runCondition && !emgFrameFromQueue.data.empty();
            if (!TimeCompare::less(emgTime, lmtMaTime)) firstLmtArrived = true;
            if (!firstLmtArrived && runCondition) {
                subject_.setTime(emgTime);
                subject_.setEmgs(emgFrameFromQueue.data);
                subject_.updateActivations();
                subject_.pushState();
            }
        }
  
        //5. lmt, ma, emg, extTorques have been read correctly and I can push to the model
        if (runCondition) {
            subject_.setTime(emgTime);
            subject_.setEmgs(emgFrameFromQueue.data);
            subject_.setMuscleTendonLengths(lmtFrameFromQueue.data);
            for (unsigned int i = 0; i < noDof_; ++i)     
                subject_.setMomentArms(momentArmsFrameFromQueue.at(i).data, i); 
            subject_.updateState();
            subject_.pushState();
    #ifdef LOG_FILES
          //:TODO: Improve as now you are defining two times what you want to log 
          vector<double> data;
          subject_.getActivations(data); 
          ModelEvaluationBase<Logger>::logger.log(emgTime, data, "Activations");
          subject_.getFiberLengths(data);
          ModelEvaluationBase<Logger>::logger.log(emgTime, data, "FiberLenghts");
          subject_.getFiberVelocities(data); 
          ModelEvaluationBase<Logger>::logger.log(emgTime, data, "FiberVelocities");
          subject_.getMuscleForces(data); 
          ModelEvaluationBase<Logger>::logger.log(emgTime, data, "MuscleForces");
          subject_.getTorques(data);
          ModelEvaluationBase<Logger>::logger.log(emgTime, data, "Torques");
    #endif
      
    #ifdef LOG
          cout << endl << endl << "EmgTime: " << emgTime << endl << "EMG" << endl;
          for (auto& it:emgFrameFromQueue.data)
            cout << it << "\t";
       
          cout << endl << "LmtTime: " << lmtMaTime << endl << "Lmt" << endl;
          for (auto& it:lmtFrameFromQueue.data)
            cout << it << "\t"; 

          for (unsigned int j = 0; j < dofNames_.size(); ++j) {
            cout << endl << "MomentArms on: " << dofNames_.at(j) << endl;
            for (auto& it: momentArmsFrameFromQueue.at(j).data)
              cout << it << "\t"; 
          }
      
          for (auto& it:externalTorquesFrameFromQueue.data)
            cout << it << " "; 
       
          vector<double> cTorques;
          subject_.getTorques(cTorques);
          for(unsigned int i = 0; i < cTorques.size(); ++i) {
            cout << "\nCurrent Torque on " << dofNames_.at(i) << " ";
            cout << cTorques.at(i); 
          }   
          cout << endl << "----------------------------------------" << endl;
    #endif
        } 

   //   float globalTimeLimit = ModelEvaluationBase<Logger>::getGlobalTimeLimit();
    //   runCondition = TimeCompare::less(emgTime,  globalTimeLimit) && TimeCompare::less(lmtMaTime, globalTimeLimit) && runCondition;
     
 } while (runCondition);

  
  
#ifdef LOG_FILES
      //:TODO: improve this... This is the end... so I'm just writing on the queue a time equal to 0
      vector<double> endData;
      double endTime =  std::numeric_limits<double>::infinity();
      ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Activations");
      ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FiberLenghts");
      ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FiberVelocities");
      ModelEvaluationBase<Logger>::logger.log(endTime, endData, "MuscleForces");
      ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Torques");
#endif
      
      doneWithExecution();
#ifdef LOG  
   cout << "Estimation completed. " << endl;
#endif
  
   
}


