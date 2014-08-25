//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani, Elena Ceseracciu - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//         elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef QueuesToTrialData_h
#define QueuesToTrialData_h

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "OutputQueues.h"
#include "InputQueues.h"
#include "TrialData.h"

class QueuesToTrialData {
public:
    QueuesToTrialData() = delete;
    QueuesToTrialData(const QueuesToTrialData&)= delete;
    QueuesToTrialData& operator=(const QueuesToTrialData&) = delete;

    template<typename NMSmodel> QueuesToTrialData(CEINMS::InputConnectors& inputConnectors, NMSmodel& subject, std::string id) : inputConnectors_(inputConnectors)
    {
        data_.id_ = id;
        subject.getMuscleNames(data_.muscleNames_);
        subject.getDoFNames(data_.dofNames_);
        subject.getMuscleNamesOnDofs(data_.maMusclesNames_);
        data_.noMuscles_ = subject.getNoMuscles();
        data_.noDoF_ = subject.getNoDoF();
        data_.torqueData_.resize(data_.noDoF_);
        data_.maData_.resize(data_.maMusclesNames_.size());
        data_.noTorqueSteps_ = data_.noLmtSteps_ = data_.noEmgSteps_ = 0;
    };
    ~QueuesToTrialData() = default;
    void operator()();
    TrialData getTrialData();

private:
    TrialData data_;
    CEINMS::InputConnectors& inputConnectors_;
    //std::vector<std::string> valuesToWrite_;
    //std::vector<std::string> torqueNames_;
    //std::vector<std::string> muscleNames_;
};

#endif