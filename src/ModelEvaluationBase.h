//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_ModelEvaluationBase_h
#define ceinms_ModelEvaluationBase_h

#include <vector>
#include <string>

#include "InputConnectors.h"
#include "OutputConnectors.h"

namespace ceinms {
    template <typename Logger>
    class ModelEvaluationBase {

    public:
        ModelEvaluationBase() = delete;
        ModelEvaluationBase(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, const std::vector<std::string>& valuesToLog);
        virtual ~ModelEvaluationBase();
        virtual void operator()() = 0;

    protected:
        void getEmgFromInputQueue(InputConnectors::FrameType& emgFromQueue);
        void getLmtFromInputQueue(InputConnectors::FrameType& lmtFromQueue);
        void getMomentArmsFromInputQueue(InputConnectors::FrameType& momentArmsFromQueue, unsigned int whichDof);
        void getExternalTorquesFromInputQueue(InputConnectors::FrameType& externalTorquesFromQueue);
        void doneWithExecution();

        InputConnectors::FrameType getEmgFromInputQueue();
        InputConnectors::FrameType getLmtFromInputQueue();
        InputConnectors::FrameType getMomentArmsFromInputQueue(unsigned int whichDof);
        InputConnectors::FrameType getExternalTorquesFromInputQueue();

        void subscribeToInputConnectors();
        bool externalTorquesAvailable() const;
        float getGlobalTimeLimit() const;

        Logger logger;

    private:
        InputConnectors& inputConnectors_;
        OutputConnectors& outputConnectors_;
    };
}

#include "ModelEvaluationBase.cpp"
#endif