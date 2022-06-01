#ifndef ceinms_BatchOpenSimEvaluator_h
#define ceinms_BatchOpenSimEvaluator_h

#include <string>
#include <vector>
#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/Result.h"
#include "ceinms/DataTable.h"
#include "ceinms/TrialData.h"


namespace ceinms {


    class BatchOpenSimEvaluator {

    public:
       BatchOpenSimEvaluator();
       BatchOpenSimEvaluator(const std::vector<TrialData>& osimTrialData);
       void setOpenSimTrialData(const std::vector<TrialData>& osimTrialData);
       void evaluate(const std::vector<Result>& resultsFromCeinms);
       std::vector<Result> getResults() const;

    private:
    //    //pass by copy because I will return a new one anyway
       static DataTable<SimTK::Vec3> evaluate(std::unique_ptr<JointContactFromOpensim>& jointContactModel, const Result& resultFromCeinms, const TrialData& osimTrialData);
       std::vector<TrialData> osimTrialData_;
       std::vector<std::unique_ptr<JointContactFromOpensim>> jointContactForcesFromOpenSim_; 
       std::vector<Result> openSimResults_;
    };

}

#endif