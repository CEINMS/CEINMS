#include <vector>
using std::vector;
#include "TrialData.h"

template<typename NMSmodelT>
ComputationMode_Default<NMSmodelT>::ComputationMode_Default(NMSmodelT& subject):
subject_(subject) { }


template<typename NMSmodelT>
void ComputationMode_Default<NMSmodelT>::setTrials( const vector<TrialData>& trials)
{

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
    
    // resizing emgDataT1_
    // forceDataT1_ forces at previous calibration step
    activationDataT1_.resize(trials_.size());
    for(unsigned int i = 0; i < trials_.size(); ++i) {
        activationDataT1_.at(i).resize(trials_.at(i).noEmgSteps_);
        for(unsigned int j = 0; j < trials_.at(i).noEmgSteps_; ++j)
            activationDataT1_.at(i).at(j).resize(nMuscles);            
    }
    
}


template<typename NMSmodelT>
void ComputationMode_Default<NMSmodelT>::getMusclesToUpdate() {

    vector<MuscleParameters> currentParameters;
    musclesToUpdate_.clear();
    subject_.getMusclesParameters(currentParameters);
    for(unsigned int i = 0; i < currentParameters.size(); ++i)
        if(!(currentParameters.at(i) == parametersT1_.at(i)))
            musclesToUpdate_.push_back(i);
    parametersT1_ = currentParameters;
}


template<typename NMSmodelT>
void ComputationMode_Default<NMSmodelT>::initFiberLengthTraceCurves(unsigned trialIndex) {
    
    unsigned const ct = trialIndex;
    unsigned k = 0; // k is the index for lmt and ma data
    double lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);
    double emgTime = trials_.at(ct).emgTimeSteps_.at(0);

    subject_.resetFibreLengthTraces(musclesToUpdate_);
    
    for (unsigned i = 0; i < trials_.at(ct).noEmgSteps_; ++i) {
    
        // set emg to model and save the activations
        subject_.setActivations(activationDataT1_.at(ct).at(i));
        emgTime = trials_.at(ct).emgTimeSteps_.at(i);
        subject_.setTime_emgs_updateActivations_pushState_selective(emgTime, trials_.at(ct).emgData_.at(i), musclesToUpdate_);
        vector<double> currentActivations;
        subject_.getActivations(currentActivations);
        for (int mi = 0; mi < subject_.getNoMuscles(); ++mi)
            activationDataT1_.at(ct).at(i).at(mi) = currentActivations.at(mi);
        
        // set lmt 
        if ( (lmtTime <= emgTime) && (k < trials_.at(ct).noLmtSteps_)) {
            subject_.setMuscleTendonLengthsSelective(trials_.at(ct).lmtData_.at(k), musclesToUpdate_);
            subject_.updateFibreLengths_OFFLINEPREP(musclesToUpdate_);
            ++k;
            if (k < trials_.at(ct).noLmtSteps_)
                lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);  
        }
    }
    subject_.updateFibreLengthTraces(musclesToUpdate_); 
}


template<typename NMSmodelT>
void ComputationMode_Default<NMSmodelT>::computeTorques(vector< vector< std::vector<double> > >& torques) {

       getMusclesToUpdate();
    // i is for the number of trials
    for (unsigned int ct = 0 ; ct < trials_.size(); ++ct) {
        
        // initializing fiber-length curves for the trial ct
        initFiberLengthTraceCurves(ct);
        
        
        int k = 0; // k is the index for lmt and ma data
        double lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);
        double emgTime = trials_.at(ct).emgTimeSteps_.at(0) + subject_.getGlobalEmDelay();

        
        // Let's start going through the EMG, lmt, and ma data...  
        for (int i = 0; i < trials_.at(ct).noEmgSteps_; ++i) {

            emgTime = trials_.at(ct).emgTimeSteps_.at(i);
            subject_.setActivations(activationDataT1_.at(ct).at(i));
            if(emgTime < lmtTime) 
                subject_.setTime_emgs_updateActivations_pushState_selective(emgTime, trials_.at(ct).emgData_.at(i), musclesToUpdate_);
//             subject_.setTime(emgTime);
//             subject_.setEmgsSelective(trials_.at(ct).emgData_.at(i), musclesToUpdate_);
//             subject_.updateActivations(musclesToUpdate_);
//             subject_.pushState(musclesToUpdate_);


            if ( (lmtTime <= emgTime) && (k < trials_.at(ct).noLmtSteps_)) {
                subject_.setTime(emgTime);
                subject_.setEmgsSelective(trials_.at(ct).emgData_.at(i), musclesToUpdate_);
                subject_.setMuscleForces(forceDataT1_.at(ct).at(k));
                subject_.setMuscleTendonLengthsSelective(trials_.at(ct).lmtData_.at(k), musclesToUpdate_); 
                for (int j = 0 ; j < trials_.at(ct).noDoF_; ++j)
                    subject_.setMomentArms(trials_.at(ct).maData_.at(j).at(k), j);
                subject_.updateState_OFFLINE(musclesToUpdate_);
                subject_.pushState(musclesToUpdate_);
                vector<double> currentTorques, currentForces;
                subject_.getTorques(currentTorques);
                subject_.getMuscleForces(currentForces);
            // when I'm done with the moment arm, I can ask for the new torque, and put it in the matrix
            for (int j = 0; j < trials_.at(ct).noDoF_; ++j)
                torques.at(ct).at(j).at(k) = currentTorques.at(j); 
            for (int j = 0; j < subject_.getNoMuscles(); ++j)
                forceDataT1_.at(ct).at(k).at(j) = currentForces.at(j);
            
            ++k;
            if (k < trials_.at(ct).noLmtSteps_)
                lmtTime = trials_.at(ct).lmtTimeSteps_.at(k);  

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
            }
            vector<double> currentActivations;
            subject_.getActivations(currentActivations);
            for (int mi = 0; mi < subject_.getNoMuscles(); ++mi)
                activationDataT1_.at(ct).at(i).at(mi) = currentActivations.at(mi);
        } 
    }
    /*
            for(unsigned ct = 0; ct < forceDataT1_.size(); ++ct)
        for(unsigned j = 0; j < forceDataT1_.at(ct).size(); ++j) {
            for(unsigned k = 0; k < forceDataT1_.at(ct).at(j).size(); ++k)
                std::cout << forceDataT1_.at(ct).at(j).at(k) << std::endl;
            std::cout << std::endl;
            
        }    */
    
}


template<typename NMSmodelT>
ComputationMode_Default<NMSmodelT>& ComputationMode_Default<NMSmodelT>::operator=(const ComputationMode_Default<NMSmodelT>& orig) {
    
    subject_         = orig.subject_;
    trials_          = orig.trials_;
    parametersT1_     = orig.parametersT1_;
    forceDataT1_      = orig.forceDataT1_;
    activationDataT1_ = orig.activationDataT1_;
    musclesToUpdate_  = orig.musclesToUpdate_;
    return *this;
}
