#ifndef ceinms_BatchContactModelEvaluator_h
#define ceinms_BatchContactModelEvaluator_h

#include <string>
#include <vector>
#include "ceinms/ContactModelKneeBalance.h"
#include "ceinms/ContactModelFromOpenSimSetup.h"
#include "ceinms/Result.h"
#include "ceinms/DataTable.h"
#include "ceinms/TrialData.h"

namespace ceinms {

class BatchContactModelEvaluator {

public:
    BatchContactModelEvaluator();
    BatchContactModelEvaluator(
        const std::vector<TrialData>& trialData, 
        const ContactModelKneeBalanceSetup& setupconst, 
        const std::vector<std::string>& dofsFromSubject);
    void evaluate(const std::vector<Result>& resultsFromCeinms);
    void initialize(const std::vector<TrialData>& trialData,
        const ContactModelKneeBalanceSetup& setupconst,
        const std::vector<std::string>& dofsFromSubject);
    void updateResults(std::vector<Result>& results) const;
private:
    //pass by copy because I will return a new one anyway
    static DataTable<double> evaluate(ContactModelKneeBalance& jointContactModel, const Result& results, const TrialData& trialData);
    std::vector<TrialData> trialData_;
    ContactModelKneeBalance contactModel_;
    std::vector<DataTable<double>> contactForcesResults_;
};

}

#endif