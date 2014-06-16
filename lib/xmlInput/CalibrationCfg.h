//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef CalibrationCfg_h
#define CalibrationCfg_h

namespace CalibrationCfg{
    
    //Objective Function
    const unsigned MinimizeTorqueError = 1000;
 
    //Parameters
    const unsigned StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single = 100;
    const unsigned ParameterSet = 200;
    
    //Torque Computation
    const unsigned Default = 10;
    const unsigned Fast = 20;
    
    //Algorithm
    const unsigned Siman = 1;
    
    
    const unsigned Nop = 0;
                
    
    enum OptimizationAlgorithm {
        
        SimulatedAnnealing = 1
    };
    
    enum Step{
        
        MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleDefault = 1110,
        MinimizeTorqueErrorStrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_singleFast = 1120,
        
        MinimizeTorqueErrorParameterSetDefault = 1210,
        MinimizeTorqueErrorParameterSetFast = 1220,
        
        Empty = -1
                 
                 };
};

#endif