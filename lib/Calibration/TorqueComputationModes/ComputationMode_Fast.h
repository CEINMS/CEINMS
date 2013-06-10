#ifndef ComputationMode_Fast_h
#define ComputationMode_Fast_h

#include <vector>
#include "TrialData.h"

/* 
 * You should use this computation mode when calibrating. It compute the model in the right way.
 * forceDataT1_ and activationDataT1_ are used to speed up the torque computation, saving the
 * force and the activation states at the previous calibration step. Works very well with
 * simulated annealing
 */

template<typename NMSmodelT>
class ComputationMode_Fast {
    
public:
    ComputationMode_Fast(NMSmodelT& subject);
    void setTrials(const std::vector<TrialData>& trials);
    ComputationMode_Fast& operator=(const ComputationMode_Fast& orig);
    void computeTorques(std::vector< std::vector< std::vector<double> > >& torques);
    void computePenalties(std::vector< std::vector< double > >& penalties);
private:
    void getMusclesToUpdate();
    void initFiberLengthTraceCurves(unsigned trialIndex);
    NMSmodelT& subject_;
    std::vector< TrialData > trials_;
    std::vector<MuscleParameters> parametersT1_;
    std::vector< std::vector < std::vector<double> > > forceDataT1_; //1st trial, 2nd row, 3rd muscle
    std::vector< std::vector < std::vector<double> > > normFiberVelDataT1_; //1st trial, 2nd row, 3rd muscle
    std::vector<unsigned> musclesToUpdate_;
    
};

#include "ComputationMode_Fast.cpp"

#endif
