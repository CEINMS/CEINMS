//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef SumMinObjectiveFunction_singleF_h
#define SumMinObjectiveFunction_singleF_h

#include <vector>
/**
 *
 * This is the basic objective function dealing with multiple DoF
 *
 * if you want to replicate and add new objective function
 * they must have an implementation of the method
 * that are called by SimulatedAnnealing class
 * 
 */

template<typename TorquesComputationT>
class SumMinObjectiveFunction_singleF {
public:
    SumMinObjectiveFunction_singleF(TorquesComputationT& torquesComputation, 
                                    double epsilon, 
                                    unsigned nEpsilon);
    void   evalFp();
    bool   isFacceptable();
    void   updateF();
    bool   isFoptAcceptable();
    void   updateFopt();
    bool   terminate();
    void   updateFandFlatest();
    void   printFs();
    double computeMetropolisCriteria(const double t);

private:
    
    void computeVariance();
    TorquesComputationT&                                   torquesComputation_;
    std::vector < std::vector < std::vector < double > > > torques_;
    std::vector < std::vector < double > >                 penalties_;
    std::vector < std::vector < std::vector < double > > > inverseTorques_;
    std::vector < std::vector < double > >                 torquesVariance_;
    
    unsigned noTrials_;
    unsigned noDoF_;
    double   fp_;
    double   f_;
    double   fOpt_;
    unsigned nEpsilon_;
    double   epsilon_;

    std::vector<double>   fLatest_;
    std::vector<unsigned> dofsIndexListToCalibrate_;

};

#include "SumMinObjectiveFunction_singleF.cpp"

#endif
