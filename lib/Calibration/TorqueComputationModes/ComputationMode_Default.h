//__________________________________________________________________________
// Author: Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ComputationMode_Default_h
#define ComputationMode_Default_h

#include <vector>
#include "TrialData.h"

//la differenza tra default e fast stan nel modo in cui viene inizializzata la curva di forza lunghezza..
/* 
 * You should use this computation mode when calibrating. It compute the model in the right way.
 * forceDataT1_ and activationDataT1_ are used to speed up the torque computation, saving the
 * force and the activation states at the previous calibration step. Works very well with
 * simulated annealing
 */

template<typename NMSmodelT>
class ComputationMode_Default {
  
public:
    ComputationMode_Default(NMSmodelT& subject);
    void setTrials(const std::vector<TrialData>& trials);
    ComputationMode_Default& operator=(const ComputationMode_Default& orig);
    void computeTorques(std::vector< std::vector< std::vector<double> > >& torques);
    void computeTorquesAndPenalties(std::vector< std::vector< std::vector<double> > >& torques, std::vector< std::vector< double > >& penalties);
private:
    void getMusclesToUpdate();
    void initFiberLengthTraceCurves(unsigned trialIndex);
    NMSmodelT& subject_;
    std::vector< TrialData > trials_;
    std::vector<MuscleParameters> parametersT1_;
    std::vector< std::vector < std::vector<double> > > forceDataT1_; //1st trial, 2nd row, 3rd muscle
    std::vector< std::vector < std::vector<double> > > penaltyDataT1_; //1st trial, 2nd row, 3rd muscle
    std::vector< std::vector < std::vector<double> > > activationDataT1_;   //1st trial, 2nd row, 3rd muscle
    std::vector<unsigned> musclesToUpdate_;
};

#include "ComputationMode_Default.cpp"

#endif
