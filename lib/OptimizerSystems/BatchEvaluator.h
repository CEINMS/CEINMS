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

        DataTable<double> forces, torques, activations, penalties;
        Result(size_t nMTUs, size_t nDofs, size_t nEmgs, size_t nLmtMa) :
            forces(nLmtMa, nMTUs),
            torques(nLmtMa, nDofs),
            activations(nEmgs, nMTUs),
            penalties(nLmtMa, nMTUs) {}
        bool equals(const Result& rhs) const {
            return (forces      == rhs.forces      &&
                    torques     == rhs.torques     &&
                    activations == rhs.activations &&
                    penalties   == rhs.penalties);

        }

    };

    bool operator==(const Result& lhs, const Result& rhs) {
        return lhs.equals(rhs);
    }


    class OpenLoopEvaluator {
    public:
        OpenLoopEvaluator() = delete;
        template<typename NMSmodelT>
        static Result evaluate(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result previousResult);
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