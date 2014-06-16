//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include <math.h>
#include <float.h>
#include <time.h>
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

//#define LOG_SIMULATED_ANNEALING
//#define DEBUG

template<typename TorquesComputationT>
SumMinObjectiveFunction_singleF<TorquesComputationT>
::SumMinObjectiveFunction_singleF(TorquesComputationT& torquesComputation, 
                                  double epsilon, 
                                  unsigned int nEpsilon)
:torquesComputation_(torquesComputation), epsilon_(epsilon), nEpsilon_(nEpsilon) { 
    
    torquesComputation_.resizeTorquesVector(torques_);
    torquesComputation_.resizePenaltiesVector(penalties_);
    torquesComputation_.setInverseTorques(inverseTorques_);
    torquesComputation_.getDofsToCalibrateIndexList(dofsIndexListToCalibrate_);
    noTrials_ = torques_.size();
    noDoF_ = dofsIndexListToCalibrate_.size();	

#ifdef DEBUG
    cout << "noDof " << noDoF_ << endl;
    cout << "dofs index list ";
    for(unsigned int i = 0; i < noDoF_; ++i)
        cout << dofsIndexListToCalibrate_.at(i) << " ";
    cout << endl;
    cout << "noTrials " << noTrials_ << endl;
    cout << "noFrames ";
    for(unsigned int i = 0; i < noTrials_; ++i)   
        cout << torques_.at(i).at(0).size() << " ";
    cout << endl; 
#endif

    torquesVariance_.resize(noTrials_);
    for (unsigned int t = 0; t < noTrials_; ++t)
        torquesVariance_.at(t).resize(noDoF_);
    // inizialize f_ and fOpt_ at the max value
    f_= fOpt_ = DBL_MAX;
    fLatest_.resize(nEpsilon_);
    for (unsigned int a = 0; a < nEpsilon_; ++a)
        fLatest_.at(a) = DBL_MAX; 
    computeVariance(); 
}
 
 
template<typename TorquesComputationT>
void SumMinObjectiveFunction_singleF<TorquesComputationT>::computeVariance( ) {
//torquesVariance is a matrix: first dimension is the trial, second dimension is the DoF
// therefore torquestVariance.at(ct).at(k) means the variance in the moment of
// k DoF for the trial ct
    unsigned int d;
    for (unsigned int dIndex = 0; dIndex < noDoF_; ++dIndex) {
        d = dofsIndexListToCalibrate_.at(dIndex);
        for (unsigned int t = 0; t < noTrials_; ++t) {
            int noDataRows = inverseTorques_.at(t).at(d).size();
            double sum = 0.;
            for (int r = 0; r < noDataRows; ++r) 
                sum += inverseTorques_.at(t).at(d).at(r);
            double average = sum/noDataRows;
            torquesVariance_.at(t).at(dIndex) = 0.;
            double ep = 0.;
            for (int r = 0; r < noDataRows; ++r) {
                torquesVariance_.at(t).at(dIndex) += 
                    (inverseTorques_.at(t).at(d).at(r) - average) * 
                    (inverseTorques_.at(t).at(d).at(r) - average);
                ep += (inverseTorques_.at(t).at(d).at(r) - average);
            }
// calculate variance correcting for roundoff error
            torquesVariance_.at(t).at(dIndex) = (torquesVariance_.at(t).at(dIndex) - ep*ep/noDataRows)/(noDataRows-1);
        }
    }
}


template<typename TorquesComputationT>
void SumMinObjectiveFunction_singleF<TorquesComputationT>::evalFp( ) {

// get the torques
    torquesComputation_.computeTorquesAndPenalties(torques_, penalties_);
// now I eval fp which is an array
// each member is the fp for a different dof
// and is the difference between computed and experimental torques
// for all the trials and data rows

// reset fp_
    fp_ = 0;
    unsigned int d;
    // first dimension: the trial
    for (unsigned int t = 0; t < noTrials_; ++t) {
        // second dimension: the DoF
        for (unsigned int dIndex = 0; dIndex < noDoF_; ++dIndex) {
            d = dofsIndexListToCalibrate_.at(dIndex);
            double trialDifference = 0;
            double penaltyFactor = 0;
            // third dimension: the dat
            for (unsigned int r = 0; r <  torques_.at(t).at(d).size() ; ++r) {
                trialDifference += ( ( torques_.at(t).at(d).at(r) - inverseTorques_.at(t).at(d).at(r) ) * 
                                     ( torques_.at(t).at(d).at(r) - inverseTorques_.at(t).at(d).at(r) ) );
                penaltyFactor += penalties_.at(t).at(r);
            }
 //           penaltyFactor = 0;
 //           if (penaltyFactor > 0)
 //               cout << "penalty factor: " << penaltyFactor << endl; 
            trialDifference = (trialDifference/torquesVariance_.at(t).at(dIndex)+penaltyFactor) / torques_.at(t).at(d).size();
            fp_ += trialDifference;
        }
    }
}

// if (fp_ <= f_)

template<typename TorquesComputationT>
bool SumMinObjectiveFunction_singleF<TorquesComputationT>::isFacceptable() {
    
    if ( fp_ > f_ )
        return false;
    return true;
}


template<typename TorquesComputationT>
void SumMinObjectiveFunction_singleF<TorquesComputationT>::updateF() {
    
    f_ = fp_;
}

 //if (fp_ < fOpt_) {

template<typename TorquesComputationT>
bool SumMinObjectiveFunction_singleF<TorquesComputationT>::isFoptAcceptable() {
    
    if ( fp_ > fOpt_ )
        return false;
    return true;
}


template<typename TorquesComputationT>
void SumMinObjectiveFunction_singleF<TorquesComputationT>::updateFopt() {
    
  fOpt_ = fp_;
}


template<typename TorquesComputationT>
void SumMinObjectiveFunction_singleF<TorquesComputationT>::printFs() {
    
    cout << "fp_: ";
    cout << fp_ << " ";
    cout << endl;
    cout << "f_: ";
    cout << f_ << " ";
    cout << endl;
    cout << "fOpt_: ";
    cout << fOpt_ << " ";
    cout << endl; 
}

// computation of Metropolis criteria
// p = exp(f-fp)/T
// is now, for multiple DoF:
// p = exp(averageOnAllDoF(f-fp)/T)

template<typename TorquesComputationT>
double SumMinObjectiveFunction_singleF<TorquesComputationT>::computeMetropolisCriteria(const double t) {
    double p = (f_ -fp_);
 
#ifdef LOG_SIMULATED_ANNEALING
    cout << "f_ " << f_ << " fp_ " << fp_  << endl;
#endif
    
    return exp(p/t);
}


// return TRUE when simulated annealing has
// converged
// if change in fopt < epsilon in the latest Nepsilon iteration

template<typename TorquesComputationT>
bool SumMinObjectiveFunction_singleF<TorquesComputationT>::terminate() {
// updated fLatest.at(0) with the latest computed value
  
#ifdef LOG_SIMULATED_ANNEALING
  cout << "TERMINATE: fOpt_ ";
    cout << fOpt_ << " ";
  cout << endl;
#endif
  
  fLatest_.at(0) = f_;
  
#ifdef LOG_SIMULATED_ANNEALING
  for (unsigned int a = 0; a < nEpsilon_; ++a) {
    cout << "TERMINATE: fLatest_[" << a << "]: ";
       cout << fLatest_.at(a) << " ";
    cout << endl;
  }
#endif
  
    for (unsigned int a = 0; a < nEpsilon_; ++a) {
    // we check that each DoF had a change in fOpt less than epsilon
        if ( fabs( fLatest_.at(a) - fOpt_ ) > epsilon_ ) {
            cout << "Return false" << endl;
            cout << "fLatest_.at(" << a << ") - fOpt_ = "
                 << fLatest_.at(a) << " - " << fOpt_ << " > " 
                 << epsilon_ << endl;
            return false;
        } 
    }
    cout << "Return true! " << endl;
    cout << "fOpt_ = " << fOpt_ << endl;
    return true;
}



template<typename TorquesComputationT>
void SumMinObjectiveFunction_singleF<TorquesComputationT>::updateFandFlatest() {
  for (unsigned int a = 1; a < nEpsilon_; ++a) 
		fLatest_.at(nEpsilon_-a) = fLatest_.at(nEpsilon_-a-1);
	
	f_ = fOpt_;	
}
