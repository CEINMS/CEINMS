//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani, Elena Ceseracciu - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//         elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "QueuesToTrialData.h"
#include "InputQueues.h"


void QueuesToTrialData::operator()() {

    CEINMS::InputConnectors::queueLmt.subscribe();
    CEINMS::InputConnectors::queueEmg.subscribe();
    for (auto& it : CEINMS::InputConnectors::queueMomentArms)
        (*it).subscribe();
    CEINMS::InputConnectors::queueExternalTorques.subscribe();

    CEINMS::InputConnectors::doneWithSubscription.wait();

    CEINMS::OutputConnectors::doneWithExecution.wait();

    double externalTorqueTime = std::numeric_limits<double>::lowest();
    bool lmtRunning = true;
    bool torqueRunning = true;
    bool emgRunning = true;
    do {  // while(runCondition)

        // 1. read lmt Data
        if (lmtRunning)
        {
            CEINMS::InputConnectors::FrameType lmtFrameFromQueue = CEINMS::InputConnectors::queueLmt.pop();
            if (lmtFrameFromQueue.time < std::numeric_limits<double>::infinity())
            {
                data_.lmtTimeSteps_.push_back(lmtFrameFromQueue.time);
                data_.lmtData_.push_back(lmtFrameFromQueue.data);
                data_.noLmtSteps_++;
                // 2. read moment arms data
                for (unsigned int i = 0; i < CEINMS::InputConnectors::queueMomentArms.size(); ++i) {
                    CEINMS::InputConnectors::FrameType momentArmsFrameFromQueue = (*CEINMS::InputConnectors::queueMomentArms.at(i)).pop();
                    data_.maData_.at(i).push_back(momentArmsFrameFromQueue.data);
                }
            }
            else
                lmtRunning = false;
        }

        // 3. read external Torque
        if (torqueRunning)
        {
            CEINMS::InputConnectors::FrameType externalTorquesFrameFromQueue = CEINMS::InputConnectors::queueExternalTorques.pop();
            if (externalTorquesFrameFromQueue.time < std::numeric_limits<double>::infinity())
            {
                data_.torqueTimeSteps_.push_back(externalTorquesFrameFromQueue.time);
                data_.noTorqueSteps_++;
                for (size_t iDof = 0; iDof < externalTorquesFrameFromQueue.data.size(); ++iDof)
                    data_.torqueData_.at(iDof).push_back(externalTorquesFrameFromQueue.data.at(iDof));
            }
            else
                torqueRunning = false;
        }

        if (emgRunning)
        {

            CEINMS::InputConnectors::FrameType emgFrameFromQueue = CEINMS::InputConnectors::queueEmg.pop();
            if (emgFrameFromQueue.time < std::numeric_limits<double>::infinity())
            {
                data_.emgTimeSteps_.push_back(emgFrameFromQueue.time);
                data_.emgData_.push_back(emgFrameFromQueue.data);
                data_.noEmgSteps_++;
            }
            else
                emgRunning = false;
        }

    } while (lmtRunning || emgRunning || torqueRunning);
}

TrialData QueuesToTrialData::getTrialData()
{
    return data_;
}

