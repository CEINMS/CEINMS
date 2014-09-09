#include <vector>
using std::vector;
#include "TrialData.h"
#include "TimeCompare.h"
//#define DEBUG

template<typename NMSmodelT>
ComputationMode_Fast<NMSmodelT>::ComputationMode_Fast(NMSmodelT& subject):
subject_(subject) {

    //with the automatic undersampling of emgs, time difference is negligibile compared to the default version - CP, Aug 2014
    std::cout << "CalibrationMode Fast is disabled" << std::endl;
    exit(EXIT_FAILURE);
}


template<typename NMSmodelT>
void ComputationMode_Fast<NMSmodelT>::setTrials( const vector<TrialData>& trials) {

    trials_ = trials;
    unsigned nMuscles = subject_.getNoMuscles();
    parametersT1_.assign(nMuscles, MuscleParameters());
    musclesToUpdate_.assign(nMuscles, true);
    
    
    // resizing forceDataT1_
    // forceDataT1_ forces at previous calibration step
    forceDataT1_.resize(trials_.size());
    for(unsigned int i = 0; i < trials_.size(); ++i) {
        forceDataT1_.at(i).resize(trials_.at(i).noLmtSteps_);
        for(unsigned int j = 0; j < trials_.at(i).noLmtSteps_; ++j)
            forceDataT1_.at(i).at(j).resize(nMuscles);
    }
    
    penaltyDataT1_.resize(trials_.size());
    for(unsigned int i = 0; i < trials_.size(); ++i) {
        penaltyDataT1_.at(i).resize(trials_.at(i).noLmtSteps_);
        for(unsigned int j = 0; j < trials_.at(i).noLmtSteps_; ++j)
            penaltyDataT1_.at(i).at(j).resize(nMuscles);
    }
     
    normFiberVelDataT1_.resize(trials_.size());
    for(unsigned int i = 0; i < trials_.size(); ++i) {
        normFiberVelDataT1_.at(i).resize(trials_.at(i).noLmtSteps_);
        for(unsigned int j = 0; j < trials_.at(i).noLmtSteps_; ++j)
            normFiberVelDataT1_.at(i).at(j).resize(nMuscles);
    }
}


template<typename NMSmodelT>
void ComputationMode_Fast<NMSmodelT>::getMusclesToUpdate() {

    vector<MuscleParameters> currentParameters;
    musclesToUpdate_.clear();
    subject_.getMusclesParameters(currentParameters);
    for(unsigned int i = 0; i < currentParameters.size(); ++i)
        if(!(currentParameters.at(i) == parametersT1_.at(i)))
            musclesToUpdate_.push_back(i);
    parametersT1_ = currentParameters;
}


template<typename NMSmodelT>
void ComputationMode_Fast<NMSmodelT>::initFiberLengthTraceCurves(unsigned trialIndex) {
    
    unsigned const ct = trialIndex;
    subject_.resetFibreLengthTraces(musclesToUpdate_);
    
    for (unsigned j = 0; j <  trials_.at(ct).lmtTimeSteps_.size(); ++j) {
        double lmtTime = trials_.at(ct).lmtTimeSteps_.at(j);
        subject_.setTime(lmtTime);
        subject_.setMuscleTendonLengthsSelective(trials_.at(ct).lmtData_.at(j), musclesToUpdate_);
        subject_.updateFibreLengths_OFFLINEPREP(musclesToUpdate_);
    }
    subject_.updateFibreLengthTraces(musclesToUpdate_); 
}

template<typename NMSmodelT>
void ComputationMode_Fast<NMSmodelT>::computeTorques(vector< vector< std::vector<double> > >& torques) {

    getMusclesToUpdate();
    // i is for the number of trials
    for (unsigned int ct = 0 ; ct < trials_.size(); ++ct) {
        initFiberLengthTraceCurves(ct);
        unsigned k = 0; // k is the index for lmt and ma data
        double lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);

        for (int i = 0; i < trials_.at(ct).noEmgSteps_; ++i) {
            double emgTime = trials_.at(ct).emgTimeSteps_.at(i) + subject_.getGlobalEmDelay();

// No more emg without corresponding lmt
//            if(emgTime < lmtTime) 
//                subject_.setTime_emgs_updateActivations_pushState_selective(emgTime, trials_.at(ct).emgData_.at(i), musclesToUpdate_);

            if (NumCompare::lessEqual(lmtTime, emgTime) && (k < trials_.at(ct).noLmtSteps_) && (k < trials_.at(ct).maData_.front().size())) {
                subject_.setMuscleForces(forceDataT1_.at(ct).at(k));
                subject_.setTime(emgTime);
                subject_.setEmgsSelective(trials_.at(ct).emgData_.at(i), musclesToUpdate_);
                subject_.setMuscleTendonLengths(trials_.at(ct).lmtData_.at(k));
                for (int j = 0 ; j < trials_.at(ct).noDoF_; ++j)
                    subject_.setMomentArms(trials_.at(ct).maData_.at(j).at(k), j);
   
                subject_.updateState_OFFLINE(musclesToUpdate_);
                subject_.pushState(musclesToUpdate_);
                vector<double> currentTorques, currentForces;
                subject_.getTorques(currentTorques);
                subject_.getMuscleForces(currentForces);
                for (int j = 0; j < trials_.at(ct).noDoF_; ++j)
                    torques.at(ct).at(j).at(k) = currentTorques.at(j); 
                for (int j = 0; j < subject_.getNoMuscles(); ++j)
                    forceDataT1_.at(ct).at(k).at(j) = currentForces.at(j);
            

#ifdef DEBUG
                cout << endl << endl << "EmgTime: " << emgTime << endl << "EMG" << endl;
        
                for(unsigned int l=0; l < trials_.at(ct).emgData_.at(i).size(); ++l)
                    cout << trials_.at(ct).emgData_.at(i).at(l) << "\t" ;
                cout << endl << "LmtTime: " << lmtTime << endl;
                
                for(unsigned int l=0; l < trials_.at(ct).lmtData_.at(k).size(); ++l)
                    cout << trials_.at(ct).lmtData_.at(k).at(l) << "\t";
                cout << endl << "MomentArms" << endl;
                
                for(unsigned int l=0; l < trials_.at(ct).maData_.at(0).at(k).size(); ++l)
                    cout << trials_.at(ct).maData_.at(0).at(k).at(l) << "\t";
                cout << "\ncurrent torque: " << currentTorques.at(0);
        
                cout << endl << "----------------------------------------" << endl;
#endif
                ++k;
                if (k < trials_.at(ct).noLmtSteps_)
                    lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);

            }
            
        } 
    }
    
  
}


template<typename NMSmodelT>
void ComputationMode_Fast<NMSmodelT>::computeTorquesAndPenalties(vector< vector< vector<double> > >& torques, vector< vector< double > >& penalties) {
//at the moment this is a copy (slightly modified) of the  computeTorques function above.. think about refactoring 
    
    getMusclesToUpdate();
    // i is for the number of trials
    for (unsigned int ct = 0 ; ct < trials_.size(); ++ct) {
     //   std::cout << "Trial " << ct + 1 << "/" << trials_.size() << std::endl;
        initFiberLengthTraceCurves(ct);
        unsigned k = 0; // k is the index for lmt and ma data
        double lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);

        for (int i = 0; i < trials_.at(ct).noEmgSteps_; ++i) {
            double emgTime = trials_.at(ct).emgTimeSteps_.at(i) + subject_.getGlobalEmDelay();
// No more emg without corresponding lmt
//            if(emgTime < lmtTime) 
//                subject_.setTime_emgs_updateActivations_pushState_selective(emgTime, trials_.at(ct).emgData_.at(i), musclesToUpdate_);

            if (TimeCompare::lessEqual(lmtTime, emgTime) && (k < trials_.at(ct).noLmtSteps_) && (k < trials_.at(ct).maData_.front().size())) {
                subject_.setMuscleForces(forceDataT1_.at(ct).at(k));
                subject_.setTime(emgTime);
                subject_.setEmgsSelective(trials_.at(ct).emgData_.at(i), musclesToUpdate_);
                subject_.setMuscleTendonLengths(trials_.at(ct).lmtData_.at(k));
                for (int j = 0 ; j < trials_.at(ct).noDoF_; ++j)
                    subject_.setMomentArms(trials_.at(ct).maData_.at(j).at(k), j);
                subject_.updateState_OFFLINE(musclesToUpdate_);
                subject_.pushState(musclesToUpdate_);
                vector<double> currentTorques, currentForces;
                subject_.getTorques(currentTorques);
                subject_.getMuscleForces(currentForces);
                for (int j = 0; j < trials_.at(ct).noDoF_; ++j)
                    torques.at(ct).at(j).at(k) = currentTorques.at(j); 
                for (int j = 0; j < subject_.getNoMuscles(); ++j)
                    forceDataT1_.at(ct).at(k).at(j) = currentForces.at(j);

                //calcolo delle penalty modificato per accomodare l'update di un sottoinsieme di muscoli
                vector<double> penaltiesAtT;
                subject_.getMusclesPenaltyVector(penaltiesAtT);
                for (vector<unsigned>::const_iterator it(musclesToUpdate_.begin()); it < musclesToUpdate_.end(); ++it)
                    penaltyDataT1_.at(ct).at(k).at(*it) = penaltiesAtT.at(*it);
                penalties.at(ct).at(k) = 0;
                for(unsigned j = 0; j < subject_.getNoMuscles(); ++j)
                    penalties.at(ct).at(k) += penaltyDataT1_.at(ct).at(k).at(j);
                //fine calcolo penalties
           
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
                ++k;
                if (k < trials_.at(ct).noLmtSteps_)
                    lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);  
    
            }
            
        } 
    }
}


template<typename NMSmodelT>
ComputationMode_Fast<NMSmodelT>& ComputationMode_Fast<NMSmodelT>::operator=(const ComputationMode_Fast<NMSmodelT>& orig) {
    
    subject_          = orig.subject_;
    trials_           = orig.trials_;
    parametersT1_     = orig.parametersT1_;
    forceDataT1_      = orig.forceDataT1_;
    normFiberVelDataT1_ = orig.normFiberVelDataT1_;
    musclesToUpdate_  = orig.musclesToUpdate_;
    return *this;
}

