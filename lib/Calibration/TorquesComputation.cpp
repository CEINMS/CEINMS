//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "TrialData.h"

#include <time.h>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <algorithm> //for sorting only
#include <iostream>
using std::cout;
using std::endl;


#include "NMSmodel.h" 

//#define DEBUG

template <typename ComputationModeT, typename NMSmodelT>
TorquesComputation<ComputationModeT, NMSmodelT>::TorquesComputation(NMSmodelT& subject,  
                                                                    const std::vector<TrialData>& trials, 
                                                                    const std::vector<std::string>& dofsToCalibrate)
:subject_(subject), dofsToCalibrate_(dofsToCalibrate), computationMode_(subject), trials_(trials) 
{
    computationMode_.setTrials(trials_);
}


template <typename ComputationModeT, typename NMSmodelT>
void TorquesComputation<ComputationModeT, NMSmodelT>::resizeTorquesVector(std::vector< std::vector< std::vector< double > > >& torques) {
    
    torques.resize(trials_.size());  
    for (unsigned int i = 0; i < trials_.size(); ++i) {
        torques.at(i).resize(trials_.at(i).noDoF_);
        for (int j = 0; j < trials_.at(i).noDoF_; ++j)
            torques.at(i).at(j).resize(trials_.at(i).noLmtSteps_);
    }  
}

template <typename ComputationModeT, typename NMSmodelT>
void TorquesComputation<ComputationModeT, NMSmodelT>::resizeMusclesVector(std::vector< std::vector< std::vector< double > > >& musclesVector) {
 
    musclesVector.resize(trials_.size());  
    for (unsigned int i = 0; i < trials_.size(); ++i) {
        musclesVector.at(i).resize(trials_.at(i).noMuscles_);
        for (int j = 0; j < trials_.at(i).noMuscles_; ++j)
            musclesVector.at(i).at(j).resize(trials_.at(i).noLmtSteps_);
    }      
}


template <typename ComputationModeT, typename NMSmodelT>
void TorquesComputation<ComputationModeT, NMSmodelT>::resizePenaltiesVector(std::vector<  std::vector< double  > >& penalties) {
   
    penalties.resize(trials_.size());  
    for (unsigned int i = 0; i < trials_.size(); ++i) 
        penalties.at(i).resize(trials_.at(i).noLmtSteps_);
}


template <typename ComputationModeT, typename NMSmodelT>
void TorquesComputation<ComputationModeT, NMSmodelT>::setInverseTorques(std::vector< std::vector< std::vector< double > > >& inverseTorques) {

    inverseTorques.resize(trials_.size());  
    for (unsigned int i = 0; i < trials_.size(); ++i) {
        inverseTorques.at(i).resize(trials_.at(i).noDoF_);
        for (int k = 0; k < trials_.at(i).noDoF_; ++k)
            inverseTorques.at(i).at(k) = trials_.at(i).torqueData_.at(k);
    }
}


template <typename ComputationModeT, typename NMSmodelT>
void TorquesComputation<ComputationModeT, NMSmodelT>::getMusclesIndexFromDofs(std::vector<unsigned int>& musclesIndexList) {

    subject_.getMusclesIndexFromDofs(musclesIndexList, trials_.at(0).dofNames_);
    
}

/**
 * We compute the torques for all the trials
 * and, for each trial, for all the DoF
 * and, for each DoF, for all time instants
 */
template <typename ComputationModeT, typename NMSmodelT>
void TorquesComputation<ComputationModeT, NMSmodelT>::computeTorquesAndPenalties(std::vector< std::vector< std::vector< double > > >& torques, 
                                                  std::vector< std::vector< double > >& penalties) {

 //   computationMode_.computeTorques(torques);
    computationMode_.computeTorquesAndPenalties(torques, penalties);
} 


template <typename ComputationModeT, typename NMSmodelT>
void TorquesComputation<ComputationModeT, NMSmodelT>::getDofsToCalibrateIndexList(vector<unsigned int>& dofsIndexList) {

    unsigned int k = 0;
    dofsIndexList.clear();
    for(unsigned int j = 0; j <  trials_.at(0).dofNames_.size(); ++j)
        for(unsigned int k = 0;  k < dofsToCalibrate_.size(); ++k)
            if(trials_.at(0).dofNames_.at(j) == dofsToCalibrate_.at(k))
                dofsIndexList.push_back(j);
}


template <typename ComputationModeT, typename NMSmodelT>
std::ostream& operator<< (std::ostream& output, const TorquesComputation<ComputationModeT, NMSmodelT>& tc) {

    output << "No. of Trials: " << tc.trials_.size() << endl;   
    for (unsigned int i = 0; i < tc.trials_.size(); ++i) {
        output << "Trial no. " << tc.trials_.at(i).id_ << endl;   
        output << "EMG Data" << endl;
        for (int j = 0; j < tc.trials_.at(i).noEmgSteps_; ++j) {
            output << tc.trials_.at(i).emgTimeSteps_.at(j) << " ";
            for (int k = 0; k < tc.trials_.at(i).noMuscles_; ++k)
                output << tc.trials_.at(i).emgData_.at(j).at(k) << " ";
            output << endl;  
        }
    
        output << "LMT Data" << endl;
        for (int j = 0; j < tc.trials_.at(i).noLmtSteps_; ++j) {
            output << tc.trials_.at(i).lmtTimeSteps_.at(j) << " ";
            for (int k = 0; k < tc.trials_.at(i).noMuscles_; ++k)
                output << tc.trials_.at(i).lmtData_.at(j).at(k) << " ";
            output << endl;  
        }
        
        output << "MA Data" << endl;
        for (int z = 0; z < tc.trials_.at(i).noDoF_; ++z) {
            output << "DOF: " << tc.trials_.at(i).dofNames_.at(z) << endl;
            for (int j = 0; j < tc.trials_.at(i).noLmtSteps_; ++j) {
                output << tc.trials_.at(i).lmtTimeSteps_.at(j) << " ";
                // the number of MA data depends on the available muscles
                for (unsigned int k = 0; k < tc.trials_.at(i).maData_.at(z).at(j).size(); ++k)
                    output << tc.trials_.at(i).maData_.at(z).at(j).at(k) << " ";
                output << endl;  
            }
        }
        
        output << "TORQUE Data" << endl;
        for ( int z = 0; z < tc.trials_.at(i).noDoF_; ++z ) {
            output << "DOF: " << tc.trials_.at(i).dofNames_.at(z) << endl;
            for (unsigned int j = 0; j < tc.trials_.at(i).noTorqueSteps_; ++j) {
                output << tc.trials_.at(i).lmtTimeSteps_.at(j) << " ";
                output << tc.trials_.at(i).torqueData_.at(z).at(j) << endl;
            }
        }
    }  
    return output;
}
