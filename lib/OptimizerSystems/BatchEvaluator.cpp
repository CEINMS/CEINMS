#include "BatchEvaluator.h"
#include <vector>

namespace CEINMS {

    BatchEvaluator::BatchEvaluator(const std::vector<TrialData>& trials) :
        trials_(trials) {
        
        size_t nTrials;
        for (size_t i(0); i < nTrials; ++i) {
            size_t nEmgRows = trials.at(i).emgData.getNRows();
            size_t nLmtRows = trials.at(i).lmtData.getNRows();
            size_t nDofs = trials.at(i).torqueData.getNColumns();
            size_t nMTUs = trials.at(i).lmtData.getNColumns();
            results_.emplace_back(Result(nMTUs, nDofs, nEmgRows, nLmtRows));
        }
        size_t nMusclesInSubject(trials.front().lmtData.getNColumns());
       
        //I need to resize them, otherwise I get an error at the first execution, because they are different in length
        subjectParametersT1_.resize(nMusclesInSubject);
        subjectParameters_.resize(nMusclesInSubject);

    }

    template<typename NMSmodelT>
    void BatchEvaluator::evaluate(NMSmodelT& subject) {

        subject.getMusclesParameters(subjectParameters_);
        updMusclesToUpdate();
        subjectParametersT1_ = subjectParameters_; //for next evaluation
        for (unsigned i(0); i < trials_.size(); ++i) {
            results_.at(i) = Evaluator::evaluate(subject, trials_.at(i), muscleToUpdate_.at(i), results_.at(i));
        }
    }



    void BatchEvaluator::updMusclesToUpdate() {
        musclesToUpdate_.clear();
        for (unsigned int i = 0; i < subjectParameters_.size(); ++i)
            if (!(subjectParameters_.at(i) == subjectParametersT1_.at(i)))
                musclesToUpdate_.emplace_back(i);
    }

    template<typename NMSmodelT>
    Result OpenLoopEvaluator::evaluate(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result previousResult) { //pass previousResult by copy, because I need a copy later

        //ct trial index
        initFiberLengthTraceCurves(subject, trialData, musclesToUpdate, previousResult); //pass trial or something



        unsigned lmtMaIndex(0); // k is the index for lmt and ma data
        double lmtTime = trialData.lmtData.getStartTime();
        double emgTime = trialData.emgData.getStartTime() + subject.getGlobalEmDelay();
        bool firstLmtArrived(false);

        // Let's start going through the EMG, lmt, and ma data...  
        for (unsigned emgIndex(0); emgIndex < trialData.emgData.getNRows(); ++emgIndex) {

            emgTime = trialData.emgData.getTime(emgIndex);
            if (!firstLmtArrived && emgTime < lmtTime) {
                subject.setActivations(previousResult.activations.getRow(emgIndex));
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.updateActivations(musclesToUpdate);
            }

            if (TimeCompare::lessEqual(lmtTime, emgTime) && (lmtMaIndex < trialData.lmtData_.getNRows()) && (lmtMaIndex < trialData.maData_.front().getNRows())) {
                firstLmtArrived = true;
                subject.setMuscleForces(previousResult.forces.getRow(lmtMaIndex));
                subject.setTime(emgTime);
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.setMuscleTendonLengthsSelective(trialData.lmtData.getRow(lmtMaIndex), musclesToUpdate); //might save computation by having a set muscle tendon length selective.. to check ;)
                for (unsigned dofIndex(0); dofIndex < trialData.noDoF; ++dofIndex)
                    subject.setMomentArms(trialData.maData.at(j).getRow(lmtMaIndex), j);
                subject.updateState_OFFLINE(musclesToUpdate);
                subject.pushState(musclesToUpdate);
                std::vector<double> currentTorques, currentForces;
                subject.getTorques(currentTorques);
                subject.getMuscleForces(currentForces);
                // when I'm done with the moment arm, I can ask for the new torque, and put it in the matrix
                previousResult.torques.setRow(lmtMaIndex, currentTorques);
                previousResult.forces.setRow(lmtMaIndex, currentForces);

                //calcolo delle penalty modificato per accomodare l'update di un sottoinsieme di muscoli
                std::vector<double> penaltiesAtT;
                subject_.getMusclesPenaltyVector(penaltiesAtT);

                for (auto &it : musclesToUpdate_)
                    previousResult.penalties.at(lmtMaIndex, it) = penaltiesAtT.at(it);


                //TODO: questo va spostato in obje function
                //                    penalties.at(ct).at(k) = 0;
                //                  for (unsigned j = 0; j < subject_.getNoMuscles(); ++j)
                //                    penalties.at(ct).at(k) += penaltyDataT1_.at(ct).at(k).at(j);
                //fine calcolo penalties
                ++lmtMaIndex;
                if (lmtMaIndex < trialData.lmtData.getNRows())
                    lmtTime = trialData.lmtData.getTime(lmtMaIndex);


#ifdef DEBUG
                cout << endl << endl << "EmgTime: " << emgTime << endl << "EMG" << endl;

                for (unsigned int l = 0; l < trials_.at(ct).emgData_.at(i).size(); ++l)
                    cout << trials_.at(ct).emgData_.at(i).at(l) << "\t";
                cout << endl << "LmtTime: " << lmtTime << endl;

                for (unsigned int l = 0; l < trials_.at(ct).lmtData_.at(k).size(); ++l)
                    cout << trials_.at(ct).lmtData_.at(k).at(l) << "\t";
                cout << endl << "MomentArms" << endl;

                for (unsigned int l = 0; l < trials_.at(ct).maData_.at(0).at(k).size(); ++l)
                    cout << trials_.at(ct).maData_.at(0).at(k).at(l) << "\t";
                cout << "\ncurrent torque: " << currentTorques.at(0);

                cout << endl << "----------------------------------------" << endl;
#endif
            }
            std::vector<double> currentActivations;
            subject.getActivations(currentActivations);
            previousResult.activations.setRow(emgIndex, currentActivations);


        }
        return previousResult;
    }


    template<typename NMSmodelT>
    void OpenLoopEvaluator::initFiberLengthTraceCurves(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult) {


        unsigned lmtMaIndex(0); // k is the index for lmt and ma data
        double lmtTime = trialData.lmtData.getStartTime();
        double emgTime = trialData.emgData.getStartTime() + subject.getGlobalEmDelay();
        bool firstLmtArrived(false);

        subject.resetFibreLengthTraces(musclesToUpdate);

        for (unsigned emgIndex(0); emgIndex < trialData.emgData.getNRows(); ++emgIndex) {

            emgTime = trialData.emgData.getTime(emgIndex);
            if (!firstLmtArrived && emgTime < lmtTime) {
                subject.setTime(emgTime);
                subject.setActivations(previousResult.activations.getRow(emgIndex));
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.updateActivations(musclesToUpdate);;
            }

            if (TimeCompare::lessEqual(lmtTime, emgTime) && (lmtMaIndex < trialData.lmtData_.getNRows()) && (lmtMaIndex < trialData.maData_.front().getNRows())) {
                firstLmtArrived = true;
                // set emg to model, set activations of the muscles not updated
                subject.setTime(emgTime);
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.setActivations(previousResult.activations.getRow(emgIndex));
                subject.updateActivations(musclesToUpdate);


                // set lmt 
                subject.setMuscleTendonLengthsSelective(trialData.lmtData.getRow(lmtMaIndex), musclesToUpdate);
                subject.updateFibreLengths_OFFLINEPREP(musclesToUpdate);

                //save activations for the next iteration
                vector<double> currentActivations;
                subject.getActivations(currentActivations);
                previousResult.activations.setRow(emgIndex, currentActivations);

                ++lmtMaIndex;
                if (lmtMaIndex < trialData.lmtData.getNRows())
                    lmtTime = trialData.lmtData.getTime(lmtMaIndex);
            }
        }
        subject.updateFibreLengthTraces(musclesToUpdate);
    }
}