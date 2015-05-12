/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): C. Pizzolato                                                    *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include <future>
#include <thread>

#include "ceinms/TimeCompare.h"
#include <vector>

namespace ceinms {

    BatchEvaluator::BatchEvaluator(const std::vector<TrialData>& trials) :
        trials_(trials) {

        size_t nTrials(trials.size());
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
            OpenLoopEvaluator::evaluate(subject, trials_.at(i), musclesToUpdate_, results_.at(i));
        }
    }


    template<typename NMSmodelT>
    void BatchEvaluator::evaluateParallel(NMSmodelT& subject, std::vector<NMSmodelT>& mockSubjects) {

        subject.getMusclesParameters(subjectParameters_);
        updMusclesToUpdate();
        subjectParametersT1_ = subjectParameters_; //for next evaluation
        for (auto& s : mockSubjects)
            s.setMusclesParameters(subjectParameters_);
        std::vector<std::future<void>> futures;
        for (unsigned i(0); i < trials_.size(); ++i) {
            auto fut(std::async(std::launch::async, [=, &mockSubjects](){
                OpenLoopEvaluator::evaluate(mockSubjects.at(i), trials_.at(i), musclesToUpdate_, results_.at(i));
            }));
            futures.emplace_back(std::move(fut));
        }

        for (auto& f : futures)
            f.wait();

    }

    template<typename NMSmodelT>
    void BatchEvaluator::evaluateParallel(NMSmodelT& subject) {

        subject.getMusclesParameters(subjectParameters_);
        updMusclesToUpdate();
        subjectParametersT1_ = subjectParameters_; //for next evaluation
        std::vector<std::future<void>> futures;
        for (unsigned i(0); i < trials_.size(); ++i) {
            auto fut(std::async(std::launch::async, [=](){
                OpenLoopEvaluator::evaluate(NMSmodelT(subject), trials_.at(i), musclesToUpdate_, results_.at(i)); }
            ));
            futures.emplace_back(std::move(fut));
        }

        for (auto& f : futures)
            f.wait();
    }


    void BatchEvaluator::updMusclesToUpdate() {
        musclesToUpdate_.clear();
        for (unsigned int i = 0; i < subjectParameters_.size(); ++i)
            if (!(subjectParameters_.at(i) == subjectParametersT1_.at(i)))
                musclesToUpdate_.emplace_back(i);
    }

    template<typename NMSmodelT>
    void OpenLoopEvaluator::evaluate(NMSmodelT&& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult) { //pass previousResult by copy, because I need a copy later

        const double emDelay(subject.getGlobalEmDelay());
        //ct trial index
        initFiberLengthTraceCurves(subject, trialData, musclesToUpdate, previousResult);

        unsigned lmtMaIndex(0); // k is the index for lmt and ma data
        double lmtTime = trialData.lmtData.getStartTime();
        double emgTime = trialData.emgData.getStartTime() + emDelay;
        bool firstLmtArrived(false);

        // Let's start going through the EMG, lmt, and ma data...
        for (unsigned emgIndex(0); emgIndex < trialData.emgData.getNRows(); ++emgIndex) {

            emgTime = trialData.emgData.getTime(emgIndex) + emDelay;
            if (!firstLmtArrived && TimeCompare::less(emgTime, lmtTime)) {
                subject.setActivations(previousResult.activations.getRow(emgIndex));
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.updateActivations(musclesToUpdate);
                subject.pushState(musclesToUpdate);
            }

            if (TimeCompare::lessEqual(lmtTime, emgTime) && (lmtMaIndex < trialData.lmtData.getNRows()) && (lmtMaIndex < trialData.maData.front().getNRows())) {
                firstLmtArrived = true;
                subject.setMuscleForces(previousResult.forces.getRow(lmtMaIndex));
                subject.setTime(lmtTime);

                subject.setActivations(previousResult.activations.getRow(emgIndex));
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.setMuscleTendonLengthsSelective(trialData.lmtData.getRow(lmtMaIndex), musclesToUpdate); //might save computation by having a set muscle tendon length selective.. to check ;)
             //   subject.setMuscleTendonLengths(trialData.lmtData.getRow(lmtMaIndex)); 
                for (unsigned dofIndex(0); dofIndex < trialData.noDoF; ++dofIndex)
                    subject.setMomentArms(trialData.maData.at(dofIndex).getRow(lmtMaIndex), dofIndex);
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
                subject.getMusclesPenaltyVector(penaltiesAtT);

                for (auto &it : musclesToUpdate)
                    previousResult.penalties.at(lmtMaIndex, it) = penaltiesAtT.at(it);

                ++lmtMaIndex;
                if (lmtMaIndex < trialData.lmtData.getNRows())
                    lmtTime = trialData.lmtData.getTime(lmtMaIndex);
            }
            std::vector<double> currentActivations;
            subject.getActivations(currentActivations);
            for (auto &it : musclesToUpdate)
                previousResult.activations.at(emgIndex, it) = currentActivations.at(it);




        }
    }


    template<typename NMSmodelT>
    void OpenLoopEvaluator::initFiberLengthTraceCurves(NMSmodelT&& subject, const TrialData& trialData, const std::vector<unsigned> musclesToUpdate, Result& previousResult) {

        const double emDelay(subject.getGlobalEmDelay());
        unsigned lmtMaIndex(0); // k is the index for lmt and ma data
        double lmtTime = trialData.lmtData.getStartTime();
        double emgTime = trialData.emgData.getStartTime() + emDelay;
        bool firstLmtArrived(false);

        subject.resetFibreLengthTraces(musclesToUpdate);

        for (unsigned emgIndex(0); emgIndex < trialData.emgData.getNRows(); ++emgIndex) {

            emgTime = trialData.emgData.getTime(emgIndex) + emDelay;
            if (!firstLmtArrived && TimeCompare::less(emgTime, lmtTime)) {
                subject.setTime(emgTime);
                subject.setActivations(previousResult.activations.getRow(emgIndex));
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.updateActivations(musclesToUpdate);;
            }

            if (TimeCompare::lessEqual(lmtTime, emgTime) && (lmtMaIndex < trialData.lmtData.getNRows()) && (lmtMaIndex < trialData.maData.front().getNRows())) {
                firstLmtArrived = true;
                // set emg to model, set activations of the muscles not updated
                subject.setTime(lmtTime);
                subject.setActivations(previousResult.activations.getRow(emgIndex));
                subject.setEmgsSelective(trialData.emgData.getRow(emgIndex), musclesToUpdate);
                subject.updateActivations(musclesToUpdate);

                // set lmt
                subject.setMuscleTendonLengthsSelective(trialData.lmtData.getRow(lmtMaIndex), musclesToUpdate);
                subject.updateFibreLengths_OFFLINEPREP(musclesToUpdate);
                
                //save activations for the next iteration
                std::vector<double> currentActivations;
                subject.getActivations(currentActivations);
                previousResult.activations.setRow(emgIndex, currentActivations);

                ++lmtMaIndex;
                if (lmtMaIndex < trialData.lmtData.getNRows())
                    lmtTime = trialData.lmtData.getTime(lmtMaIndex);
            }
            subject.pushState();
        }
        subject.updateFibreLengthTraces(musclesToUpdate);
    }
}
