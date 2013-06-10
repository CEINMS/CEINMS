// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com

#ifndef SimulatedAnnealing_h
#define SimulatedAnnealing_h

#include <string>
#include <vector>
#include <iostream>
//#include "NMSmodel.h"

#include "simulatedAnnealing.hxx"

// This is implementation is from the paper
// Global Optimization of Statistical Functions with Simulated Annealing
// by W. L. Goffe, G. D. Ferrier and J. Rogers
// please refer to the papers for the meaning of the variables


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT,typename NMSmodelT>
class SimulatedAnnealing;


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT,typename NMSmodelT>
std::ostream& operator<< (std::ostream&, 
                          const SimulatedAnnealing<ParametersT, 
                                                   ObjectiveFunctionT, 
                                                   TorquesComputationT, 
                                                   NMSmodelT>& sa);


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT,typename NMSmodelT>
class SimulatedAnnealing {

public:
    SimulatedAnnealing(NMSmodelT& subject, 
                       std::vector<std::string> dofsList, 
                       const std::string& configurationFile, 
                       TorquesComputationT& torquesComputation);
    //constructor for hybrid annealing
/*    SimulatedAnnealing(NMSmodel<Activation, Tendon >& subject,
                       std::vector<std::string> dofsList, 
                       StaticTorquesComputation<ComputationMode, Activation, Tendon >& staticTorquesComputation);*/
    void optimize();
    friend std::ostream& operator<< <> (std::ostream& output, const SimulatedAnnealing& sa);

private:
    void checkBounds(int k);
    
    // data from xml file for simulated annealing configuration
    std::auto_ptr<SimulatedAnnealingType> annealingPointer_;
    // input output with the subject (set/get)
    ParametersT parameters_;
    // this is implementing how we compute fp
    ObjectiveFunctionT objectiveFunction_;

    std::vector<double> x_;
    std::vector<double> upperBounds_;
    std::vector<double> lowerBounds_;
    std::vector<double> xOpt_;
    std::vector<double> xp_;
    std::vector<double> v_;
    std::vector<int>    noAccepted_; 
    int                 noParameters_;
    
//annealing parameters    
    double              nt_;
    double              ns_;
    double              rt_;
    double              t_;
    int                 maxNoEval_;  
};

#include "SimulatedAnnealing.cpp"

#endif
