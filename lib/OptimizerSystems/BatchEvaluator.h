#ifndef Ceinms_BatchEvaluator_h
#define Ceinms_BatchEvaluator_h

#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "MuscleParameters.h"
#include "DataTable.h"
#include "TrialData.h"
#include "Result.h"

namespace CEINMS {


    class OpenLoopEvaluator {
    public:
        OpenLoopEvaluator() = delete;
        template<typename NMSmodelT>
        static void evaluate(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult);
    private:
        template<typename NMSmodelT>
        static void initFiberLengthTraceCurves(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult);
    };


    class BatchEvaluator{

    public:
        using EvaluationResult = Result; //just to expose it
        BatchEvaluator(const std::vector<TrialData>& trials);
        template<typename NMSmodelT>
        void evaluate(NMSmodelT& subject);
        template<typename NMSmodelT>
        void evaluateParallel(NMSmodelT& subject);
        std::vector<Result> getResults() const {
            return results_;
        }

    private:
        void updMusclesToUpdate();
        std::vector<MuscleParameters> subjectParameters_, subjectParametersT1_; //this should be something like typename NMSmodelT::MTUtype::MuscleParameters.. since it should depend on the muscle model we are using
        std::vector<unsigned> musclesToUpdate_;
        std::vector<TrialData> trials_;
        std::vector<Result> results_;
    };    

}

#include "BatchEvaluator.cpp"
#endif