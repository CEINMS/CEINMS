// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com

#ifndef SimulatedAnnealingBase_h
#define SimulatedAnnealingBase_h

#include <string>
#include <vector>
#include <iostream>

#include "HybridWeightings.h"

// This is implementation is from the paper
// Global Optimization of Statistical Functions with Simulated Annealing
// by W. L. Goffe, G. D. Ferrier and J. Rogers
// please refer to the papers for the meaning of the variables

template<typename NMSmodelT, typename Parameters, typename ObjectiveFunction, typename StaticComputationT>
class SimulatedAnnealingBase;


template<typename NMSmodelT, typename Parameters, typename ObjectiveFunction, typename StaticComputationT>
std::ostream& operator<< (std::ostream&, const SimulatedAnnealingBase<NMSmodelT, Parameters, ObjectiveFunction, StaticComputationT>& sa);

template<typename NMSmodelT, typename Parameters, typename ObjectiveFunction, typename StaticComputationT>
class SimulatedAnnealingBase {

public:
    
    SimulatedAnnealingBase () {}
    SimulatedAnnealingBase (double nt, double ns, double rt, double t, double maxNoEval);
    SimulatedAnnealingBase( NMSmodelT& mySubject, 
                            std::vector<std::string>& muscleNamesWithEMGtoTrack,
                            std::vector<std::string>& muscleNamesWithEMGtoPredict,
                            const HybridWeightings hybridParameters,
                            StaticComputationT& staticComputation);

    void optimize();
    void getXopt(std::vector<double>& xOpt) const;
    
//    friend class SimulatedAnnealingHybrid<NMSmodelT, Parameters, ObjectiveFunction, ComputationMode>;
    friend std::ostream& operator<< <> (std::ostream& output, const SimulatedAnnealingBase& sa);

protected:
    void checkBounds(int k);
    
    Parameters parameters_;
    ObjectiveFunction objectiveFunction_;
    
    
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

#include "SimulatedAnnealingBase.cpp"

#endif
