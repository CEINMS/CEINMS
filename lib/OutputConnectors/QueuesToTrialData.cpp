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
#include "InputConnectors.h"

namespace ceinms {
    void QueuesToTrialData::operator()() {

        inputConnectors_.queueLmt.subscribe();
        inputConnectors_.queueEmg.subscribe();
        for (auto& it : inputConnectors_.queueMomentArms)
            (*it).subscribe();
        inputConnectors_.queueExternalTorques.subscribe();
        inputConnectors_.doneWithSubscription.wait();
        outputConnectors_.doneWithExecution.wait();

        if (!inputConnectors_.externalTorquesAvailable)
            return;
        double externalTorqueTime = std::numeric_limits<double>::lowest();
        bool lmtRunning = true;
        bool torqueRunning = true;
        bool emgRunning = true;
        do {  // while(runCondition)

            // 1. read lmt Data
            if (lmtRunning) {
                InputConnectors::FrameType lmtFrameFromQueue = inputConnectors_.queueLmt.pop();
                if (lmtFrameFromQueue.time < std::numeric_limits<double>::infinity()) {
                    data_.lmtData.pushRow(lmtFrameFromQueue.time, lmtFrameFromQueue.data);
                    // 2. read moment arms data
                    for (unsigned int i = 0; i < inputConnectors_.queueMomentArms.size(); ++i) {
                        InputConnectors::FrameType momentArmsFrameFromQueue = (*inputConnectors_.queueMomentArms.at(i)).pop();
                        data_.maData.at(i).pushRow(momentArmsFrameFromQueue.time, momentArmsFrameFromQueue.data);
                    }
                }
                else
                    lmtRunning = false;
            }

            // 3. read external Torque
            if (torqueRunning) {
                InputConnectors::FrameType externalTorquesFrameFromQueue = inputConnectors_.queueExternalTorques.pop();
                if (externalTorquesFrameFromQueue.time < std::numeric_limits<double>::infinity()) {
                    data_.torqueData.pushRow(externalTorquesFrameFromQueue.time, externalTorquesFrameFromQueue.data);
                }
                else
                    torqueRunning = false;
            }

            if (emgRunning) {
                InputConnectors::FrameType emgFrameFromQueue = inputConnectors_.queueEmg.pop();
                if (emgFrameFromQueue.time < std::numeric_limits<double>::infinity()) {
                    data_.emgData.pushRow(emgFrameFromQueue.time, emgFrameFromQueue.data);
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
}