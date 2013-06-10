// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com


#ifndef TorquesComputation_h
#define TorquesComputation_h

#include <iostream>

#include <vector>
#include <string>
#include "NMSmodel.h"
#include "TrialData.h"


template <typename ComputationModeT, typename NMSmodelT>
class TorquesComputation;


template <typename ComputationModeT, typename NMSmodelT>
std::ostream& operator<< (std::ostream& output, const TorquesComputation<ComputationModeT, NMSmodelT>& m); 

template <typename ComputationModeT, typename NMSmodelT>
class TorquesComputation {
  public:
    TorquesComputation(NMSmodelT& subject, 
                       const std::string& inputDataDirectory, 
                       const std::vector<std::string>& idTrials,
                       const std::vector<std::string>& dofsToCalibrate);
    void resizeTorquesVector(std::vector< std::vector< std::vector< double > > >& torques);
    void resizeMusclesVector(std::vector< std::vector< std::vector< double > > >& musclesVector);
    void resizePenaltiesVector(std::vector< std::vector< double > >& penalties);
    void computeTorquesAndPenalties(std::vector< std::vector< std::vector< double > > >& torques,
                                  std::vector< std::vector< double> >& penalties);
    void setInverseTorques(std::vector< std::vector< std::vector< double > > >& inverseTorques);
    void getDofsToCalibrateIndexList(std::vector<unsigned int>& dofsIndexList);
    void getMusclesIndexFromDofs(std::vector<unsigned int>& musclesIndexList);
    
    friend std::ostream& operator<< <> (std::ostream& output, const TorquesComputation& m); 
    
private:
    ComputationModeT computationMode_;
    NMSmodelT& subject_;
    std::vector<TrialData> trials_;
    std::vector<unsigned> musclesIndexList_;
    std::vector<std::string> dofsToCalibrate_;
};

#include "TorquesComputation.cpp"

#endif
