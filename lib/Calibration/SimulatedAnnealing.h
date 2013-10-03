//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________


#ifndef SimulatedAnnealing_h
#define SimulatedAnnealing_h

#include <string>
#include <vector>
#include <iostream>
#include "SimulatedAnnealingParameters.h"

// This is implementation is from the paper
// Global Optimization of Statistical Functions with Simulated Annealing
// by W. L. Goffe, G. D. Ferrier and J. Rogers
// please refer to the papers for the meaning of the variables


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT>
class SimulatedAnnealing;


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT>
std::ostream& operator<< (std::ostream&, 
                          const SimulatedAnnealing<ParametersT, 
                                                   ObjectiveFunctionT, 
                                                   TorquesComputationT>& sa);


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT>
class SimulatedAnnealing {

public:
    
    SimulatedAnnealing(ParametersT& parametersPolicy, TorquesComputationT& torquesComputation, SimulatedAnnealingParameters simanParameters); 
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
