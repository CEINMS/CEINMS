//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "SimpleFileLogger.h"
#include "StorageLogger.h"
#include "TimeCompare.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include "ModelEvaluationOfflineStiffness.h"
#include "ModelEvaluationBase.h"
#include "InputConnectors.h"
#include "OutputQueues.h"
#define LOG_FILES
#define LOG


template <typename NMSmodelT, typename Logger>
ModelEvaluationOfflineStiffness<NMSmodelT, Logger>::ModelEvaluationOfflineStiffness(CEINMS::InputConnectors& inputConnectors, NMSmodelT& subject, const vector<string>& valuesToLog)
    :ModelEvaluationBase<Logger>::ModelEvaluationBase(inputConnectors, valuesToLog), subject_(subject)
{
    subject_.getDoFNames(dofNames_);
    noDof_ = dofNames_.size();
    globalEmDelay_ = subject.getGlobalEmDelay();
}


template <typename NMSmodelT, typename Logger>
void ModelEvaluationOfflineStiffness<NMSmodelT, Logger>::readDataFromQueues() {
    
    bool runCondition = true;
    //read EMG data
    do{
        auto frame(ModelEvaluationBase<Logger>::getEmgFromInputQueue());
        if (!frame.data.empty())
            emgDataFromQueue_.push_back(frame);
        else runCondition = false;
    } while (runCondition);

    runCondition = true;
    //read LMT data
    do{
        auto frame(ModelEvaluationBase<Logger>::getLmtFromInputQueue());
        if (!frame.data.empty())
            lmtDataFromQueue_.push_back(frame);
        else runCondition = false;
    } while (runCondition);


    runCondition = ModelEvaluationBase<Logger>::externalTorquesAvailable();
    //read external torques
    do{
        auto frame(ModelEvaluationBase<Logger>::getExternalTorquesFromInputQueue());
        if (!frame.data.empty())
            externalTorquesDataFromQueue_.push_back(frame);
        else runCondition = false;
    } while (runCondition);


    runCondition = true;
    //read moment arms data
    do{
        vector< CEINMS::InputConnectors::FrameType > maFrame;
        for (unsigned i = 0; i < noDof_; ++i) {
            auto currentMomentArms(ModelEvaluationBase<Logger>::getMomentArmsFromInputQueue(i));
            if (!currentMomentArms.data.empty())
                maFrame.push_back(currentMomentArms);
            else runCondition = false;
        }
        if (runCondition) 
            maDataFromQueue_.push_back(maFrame);
    } while (runCondition);

}

template <typename NMSmodelT, typename Logger>
void ModelEvaluationOfflineStiffness<NMSmodelT, Logger>::initOfflineCurve() {

    auto lmtIt(lmtDataFromQueue_.begin());
    for (auto emgIt : emgDataFromQueue_) {
        double emgTime = emgIt.time + globalEmDelay_;
        subject_.setTime(emgTime);
        subject_.setEmgs(emgIt.data);
        subject_.updateActivations();
        if (TimeCompare::lessEqual(lmtIt->time, emgTime) && lmtIt != lmtDataFromQueue_.end()) {
            subject_.setMuscleTendonLengths(lmtIt->data);
            subject_.updateFibreLengths_OFFLINEPREP();
            ++lmtIt;
        }
        subject_.pushState();
    }
    subject_.updateFibreLengthTraces();
}


template <typename NMSmodelT, typename Logger>
void ModelEvaluationOfflineStiffness<NMSmodelT, Logger>::operator()() {
 
    ModelEvaluationBase<Logger>::subscribeToInputConnectors();

#ifdef LOG
    cout << "starting consume" << endl;
#endif
    readDataFromQueues();
    initOfflineCurve();

    double externalTorqueTime = std::numeric_limits<double>::lowest();
    double emgTime = std::numeric_limits<double>::lowest();
    bool runCondition = !(emgDataFromQueue_.empty() || maDataFromQueue_.empty() || lmtDataFromQueue_.empty());
    bool firstLmtArrived(false);

    while (runCondition) {  // while(runCondition)

        // 1. read lmt Data
        CEINMS::InputConnectors::FrameType lmtFrameFromQueue(lmtDataFromQueue_.front());
        lmtDataFromQueue_.pop_front();
        double lmtMaTime = lmtFrameFromQueue.time;

        // 2. read moment arms data
        vector< CEINMS::InputConnectors::FrameType > momentArmsFrameFromQueue(maDataFromQueue_.front());
        maDataFromQueue_.pop_front();

        // 3. read external Torque 
        CEINMS::InputConnectors::FrameType externalTorquesFrameFromQueue;
        while (TimeCompare::less(externalTorqueTime, lmtMaTime) && !externalTorquesDataFromQueue_.empty()) {
            externalTorquesFrameFromQueue = externalTorquesDataFromQueue_.front();
            externalTorquesDataFromQueue_.pop_front();
            externalTorqueTime = externalTorquesFrameFromQueue.time;
        }

        // 4. read emgs
        CEINMS::InputConnectors::FrameType emgFrameFromQueue;
        while (TimeCompare::less(emgTime, lmtMaTime)) {
            emgFrameFromQueue = emgDataFromQueue_.front();
            emgDataFromQueue_.pop_front();
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
        subject_.getMtusStiffness(data);
        ModelEvaluationBase<Logger>::logger.log(emgTime, data, "MtusStiffness");
        subject_.getTorques(data);
        ModelEvaluationBase<Logger>::logger.log(emgTime, data, "Torques");
        subject_.getDofsStiffness(data);
        ModelEvaluationBase<Logger>::logger.log(emgTime, data, "DofsStiffness");
#endif

#ifdef LOG
        cout << endl << endl << "EmgTime: " << emgTime << endl << "EMG" << endl;
        for (auto& it : emgFrameFromQueue.data)
            cout << it << "\t";

        cout << endl << "LmtTime: " << lmtMaTime << endl << "Lmt" << endl;
        for (auto& it : lmtFrameFromQueue.data)
            cout << it << "\t";

        for (unsigned int j = 0; j < dofNames_.size(); ++j) {
            cout << endl << "MomentArms on: " << dofNames_.at(j) << endl;
            for (auto& it : momentArmsFrameFromQueue.at(j).data)
                cout << it << "\t";
        }

        for (auto& it : externalTorquesFrameFromQueue.data)
            cout << it << " ";

        vector<double> cTorques;
        subject_.getTorques(cTorques);
        for (unsigned int i = 0; i < cTorques.size(); ++i) {
            cout << "\nCurrent Torque on " << dofNames_.at(i) << " ";
            cout << cTorques.at(i);
        }
        cout << endl << "----------------------------------------" << endl;
#endif
        runCondition = runCondition = !(emgDataFromQueue_.empty() || maDataFromQueue_.empty() || lmtDataFromQueue_.empty());
    } //end while(runCondition)
        


#ifdef LOG_FILES
    //:TODO: improve this... This is the end... so I'm just writing on the queue a time equal to 0
    vector<double> endData;
    double endTime = std::numeric_limits<double>::infinity();
    ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Activations");
    ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FiberLenghts");
    ModelEvaluationBase<Logger>::logger.log(endTime, endData, "FiberVelocities");
    ModelEvaluationBase<Logger>::logger.log(endTime, endData, "MuscleForces");
    ModelEvaluationBase<Logger>::logger.log(endTime, endData, "MtusStiffness");
    ModelEvaluationBase<Logger>::logger.log(endTime, endData, "Torques");
    ModelEvaluationBase<Logger>::logger.log(endTime, endData, "DofsStiffness");
#endif

    CEINMS::OutputConnectors::doneWithExecution.wait();
#ifdef LOG  
    cout << "Estimation completed. " << endl;
#endif

    }


template <typename NMSmodelT, typename Logger>
ModelEvaluationOfflineStiffness<NMSmodelT, Logger>::~ModelEvaluationOfflineStiffness()
{

}









