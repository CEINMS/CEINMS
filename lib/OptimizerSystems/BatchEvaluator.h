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

    struct Result{
        DataTable<double> forces, torques, activations, penalties;
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
        BatchEvaluator(const std::vector<TrialData>& trials_);
        template<typename NMSmodelT>
        void evaluate(NMSmodelT& subject);
        std::vector<Result> getResults() const;

    private:
        void updMusclesToUpdate();
        std::vector<MuscleParameters> subjectParameters_, subjectParametersT1_; //this should be something like typename NMSmodelT::MTUtype::MuscleParameters.. since it should depend on the muscle model we are using
        std::vector<unsigned> muscleToUpdate_;
        std::vector<TrialData> trials_;
        std::vector<Result> results_;
    };

    template<typename NMSmodelT>
    void BatchEvaluator::evaluate(NMSmodelT& subject) {

        subject.getMusclesParameters(subjectParameters_);
        updMusclesToUpdate();
        subjectParametersT1_ = subjectParameters_; //for next evaluation
        for (unsigned i(0); i < trials_.size(); ++i) {
            results_.at(i) = Evaluator::evaluate(subject, trials_.at(i), muscleToUpdate_.at(i), results_.at(i));
        }
    }


    template<typename NMSmodelT>
    Result Evaluator::evaluate(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result previousResult) { //pass previousResult by copy, because I need a copy later
       
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
                vector<double> currentActivations;
                subject.getActivations(currentActivations);
                previousResult.activations.setRow(emgIndex, currentActivations);


            }
        return previousResult;
        }


    template<typename NMSmodelT>
    void initFiberLengthTraceCurves(NMSmodelT& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult) {


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
#endif