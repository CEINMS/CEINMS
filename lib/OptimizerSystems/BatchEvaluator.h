#ifndef Ceinms_BatchEvaluator_h
#define Ceinms_BatchEvaluator_h

#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "MuscleParameters.h"
#include "DataTable.h"
#include "TrialData.h"

namespace CEINMS {

    class Result{
    public:

        DataTable<double> forces_, torques_, activations_, penalties_;
        Result(size_t nMTUs, size_t nDofs, size_t nEmgs, size_t nLmtMa) :
            forces_(nLmtMa, nMTUs),
            torques_(nLmtMa, nDofs),
            activations_(nEmgs, nMTUs),
            penalties_(nLmtMa, nMTUs) {}

    };



    class Evaluator {
    public:
        Evaluator() = delete;
        template<typename NMSmodelT>
        static Result evaluate(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result previousResult);
    private:
        template<typename NMSmodelT>
        static void initFiberLengthTraceCurves(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult);
    };


    class BatchEvaluator{

    public:
        BatchEvaluator(const std::vector<TrialData>& trials);
        template<typename NMSmodelT>
        void evaluate(NMSmodelT& subject);
        std::vector<Result> getResults() const;

    private:
        void updMusclesToUpdate();
        std::vector<MuscleParameters> subjectParameters_, subjectParametersT1_; //this should be something like typename NMSmodelT::MTUtype::MuscleParameters.. since it should depend on the muscle model we are using
        std::vector<unsigned> musclesToUpdate_;
        std::vector<TrialData> trials_;
        std::vector<Result> results_;
    };    
}

//#include "BatchEvaluator.cpp"
#endif