#include "ceinms/BatchContactModelEvaluator.h"


namespace ceinms {

BatchContactModelEvaluator::BatchContactModelEvaluator() {
 }

BatchContactModelEvaluator::BatchContactModelEvaluator(
    const std::vector<TrialData>& trialData, 
    const ContactModelKneeBalanceSetup& setup,
    const std::vector<std::string>& dofsFromSubject):
    trialData_(trialData),
    contactModel_(setup, dofsFromSubject) {
    
    size_t nTrials(trialData_.size());
    for (size_t i(0); i < nTrials; ++i) {
        size_t nRows(trialData_.at(i).getTorqueData().getNRows());
        size_t nCols(contactModel_.getCondyleNames().size());
        contactForcesResults_.emplace_back(DataTable<double>(nRows, nCols));
        contactForcesResults_.back().setLabels(contactModel_.getCondyleNames());
    }
}

void BatchContactModelEvaluator::initialize(
    const std::vector<TrialData>& trialData,
    const ContactModelKneeBalanceSetup& setup,
    const std::vector<std::string>& dofsFromSubject) {

    trialData_ = trialData;
    contactModel_.initialize(setup, dofsFromSubject);
    size_t nTrials(trialData_.size());
    for (size_t i(0); i < nTrials; ++i) {
        size_t nRows(trialData_.at(i).getTorqueData().getNRows());
        size_t nCols(contactModel_.getCondyleNames().size());
        contactForcesResults_.emplace_back(DataTable<double>(nRows, nCols));
        contactForcesResults_.back().setLabels(contactModel_.getCondyleNames());
    }
}

void BatchContactModelEvaluator::evaluate(const std::vector<Result>& resultsFromCeinms) {

    for (unsigned i(0); i < trialData_.size(); ++i) {
        const DataTable<double>& torqueData(trialData_.at(i).getTorqueData());
        for (unsigned r(0); r < torqueData.getNRows(); ++r) {
            contactModel_.setTorques(torqueData.getRow(r), resultsFromCeinms.at(i).torques.getRow(r));
            contactForcesResults_.at(i).setRow(r, contactModel_.getContactForces());
            contactForcesResults_.at(i).setTime(torqueData.getTime(r), r);
        }
    }
}


void BatchContactModelEvaluator::updateResults(std::vector<Result>& results) const {

    for (unsigned i(0); i < results.size(); ++i) {
        results.at(i).planarContactForces = contactForcesResults_.at(i);
    }
}

}