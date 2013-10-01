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

#include "EMGgenerator/EMGgeneratorFrom10To13.h"
#include "EMGgenerator/EMGgeneratorFrom16To34.h"
#include "EMGgenerator/EMGgeneratorFrom6To24.h"
#include "EMGDataFromFile.h"
#include "DataFromFile.h"


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
TorquesComputation<ComputationModeT, NMSmodelT>::TorquesComputation(NMSmodelT& subject, 
                       const std::string& inputDataDirectory, 
                       const std::vector<string>& idTrials,
                       const std::vector<string>& dofsToCalibrate) 
:subject_(subject), dofsToCalibrate_(dofsToCalibrate), computationMode_(subject)  
{
    // first we resize the vector that has the number of trials
    trials_.resize( idTrials.size() );
    // and we set basic data
    for (unsigned int i = 0; i < trials_.size(); ++i)
        trials_.at(i).id_ = idTrials.at(i);
   
    // For each trial
    for ( unsigned int i = 0; i < idTrials.size(); ++i ) {

    // read EMG data
        string EMGDataFilename = inputDataDirectory + idTrials.at(i) + "/emg.txt";

#ifdef DEBUG
        cout << "Reading from: " << EMGDataFilename;
#endif

        EMGDataFromFile<EMGgeneratorFrom16To34> emgDataFromFile(EMGDataFilename);
      
        if (!subject.compareMusclesNames(emgDataFromFile.getMusclesNames())) {
            cout << "Sorry! Your models have a number of muscles which is not compatible "
                 << " with the input EMG data!\n";
            exit(EXIT_FAILURE);
        }
   
        trials_.at(i).noMuscles_ = emgDataFromFile.getMusclesNames().size();
        trials_.at(i).noEmgSteps_ = emgDataFromFile.getNoTimeSteps();
       
        vector<double> nextEmgData;
        nextEmgData.resize(trials_.at(i).noMuscles_);
  
        while(emgDataFromFile.areStillData()) {
            double EMGDataTime;
            emgDataFromFile.readNextEmgData();
            EMGDataTime = emgDataFromFile.getCurrentTime();
            trials_.at(i).emgTimeSteps_.push_back(EMGDataTime);
            trials_.at(i).emgData_.push_back(emgDataFromFile.getCurrentData());
        }     

    // read LMT data
        string lmtDataFilename = inputDataDirectory + "/" + idTrials.at(i) + "/lmt.txt";

#ifdef DEBUG
        cout << "Reading from: " << lmtDataFilename;
#endif

        DataFromFile lmtDataFromFile(lmtDataFilename);
  
        if (!subject.compareMusclesNames(lmtDataFromFile.getMusclesNames())) {
            cout << "Sorry! Your models have a number of muscles which is not compatible "
                 << " with the input Lmt data!\n";
            exit(EXIT_FAILURE);
        }
            
        trials_.at(i).noLmtSteps_ =  lmtDataFromFile.getNoTimeSteps();   
        double startLmtTime = 0;
        for (int j = 0; j < trials_.at(i).noLmtSteps_; ++j) {
            lmtDataFromFile.readNextData();
            double lmtDataTime = lmtDataFromFile.getCurrentTime();
            vector<double> nextLmtData = lmtDataFromFile.getCurrentData();
            trials_.at(i).lmtTimeSteps_.push_back(lmtDataTime);
            trials_.at(i).lmtData_.push_back(nextLmtData);
        }
   
   // now, for each DoF... let us read the ma for the different muscles
        subject.getDoFNames(trials_.at(i).dofNames_);
        trials_.at(i).noDoF_ = trials_.at(i).dofNames_.size();
        trials_.at(i).maData_.resize(trials_.at(i).noDoF_);
   
        for (int k = 0; k < trials_.at(i).noDoF_; ++k ) {
            string maDataFilename = inputDataDirectory + "/" +idTrials.at(i) + "/"   
                                        + trials_.at(i).dofNames_.at(k) + "Ma.txt";
#ifdef DEBUG
            cout << "Reading from: " << maDataFilename << endl;
#endif
  
            DataFromFile maDataFromFile(maDataFilename);
            // we need to compare the Muscle Names in the DOFnameMa.txt file with
            // the muscle names of this DOF
            if (!subject.compareMusclesNamesOnDoF(maDataFromFile.getMusclesNames(), k )) {
                cout << "Sorry! Your models have a number of muscles which is not compatible "
                     << " with the input Lmt data!\n";
                exit(EXIT_FAILURE);
            }
        
            if (trials_.at(i).noLmtSteps_ != maDataFromFile.getNoTimeSteps()) {
                cout << "Sorry! You are supposed to have the same number of time step for"
                     << " lmt and ma, please check your input file\n";
                exit(EXIT_FAILURE);     
            }
        
            for (int j = 0; j < trials_.at(i).noLmtSteps_; ++j) {
                maDataFromFile.readNextData();
                vector<double> nextMaData = maDataFromFile.getCurrentData();
                trials_.at(i).maData_.at(k).push_back(nextMaData);
            }
        } // done with the k
        
        // read TORQUE data
        trials_.at(i).torqueData_.resize(trials_.at(i).noDoF_);
        for (int k = 0; k < trials_.at(i).noDoF_; ++k ) {
            string torqueDataFilename = inputDataDirectory + "/" +idTrials.at(i) + "/"   
                                        + trials_.at(i).dofNames_.at(k) + "Torque.txt";

#ifdef DEBUG
            cout << "Reading from: " << torqueDataFilename << endl;
#endif
        
            DataFromFile torqueDataFromFile(torqueDataFilename);
        
        // sorry... this is MusclesName but actually is the name of the column in Torque
        // so we compare the name of the DOF we found in the file with the
        // name we expected...
            if (trials_.at(i).dofNames_.at(k) != torqueDataFromFile.getMusclesNames().at(0)) {
                cout << "Sorry! I should work with DoF " << trials_.at(i).dofNames_.at(k)
                    << " but I found!" << torqueDataFromFile.getMusclesNames().at(0) << endl;
                exit(EXIT_FAILURE);
            }
            trials_.at(i).noTorqueSteps_ =  torqueDataFromFile.getNoTimeSteps();

            if (trials_.at(i).noLmtSteps_ != trials_.at(i).noTorqueSteps_ ) {
                cout << "Sorry! You are supposed to have the same number of time step for"
                    << " lmt and torque, please check your input file\n";
                exit(EXIT_FAILURE);     
            }
        
            for (int j = 0; j < trials_.at(i).noTorqueSteps_; ++j) {
                torqueDataFromFile.readNextData();
                double torqueDataTime = torqueDataFromFile.getCurrentTime();       
                vector<double> nextTorqueData = torqueDataFromFile.getCurrentData();
        
                if (torqueDataTime != trials_.at(i).lmtTimeSteps_.at(j)) {
                    cout << "Sorry at time " << j << " torque time step is: " 
                        << torqueDataTime << " while lmt time step is: " 
                        << trials_.at(i).lmtTimeSteps_.at(j) << endl;
                    exit(EXIT_FAILURE);
                }
            
                trials_.at(i).torqueData_.at(k).push_back(nextTorqueData.at(0));
            }     
        } // done with the k   
       
       //TODO: we have to move this from hard-coded to user defined.
     //  trials_.at(i).crop(0.0, 1.0);
        
    } // done with the trials
    subject_.getMusclesIndexFromDofs(musclesIndexList_, trials_.at(0).dofNames_);                       
    
    //initialize the computation mode
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

    computationMode_.computeTorques(torques);
    
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
