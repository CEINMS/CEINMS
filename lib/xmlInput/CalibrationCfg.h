//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_CalibrationCfg_h
#define ceinms_CalibrationCfg_h

namespace ceinms {
    namespace CalibrationCfg{

        //Objective Function
        const unsigned MinimizeTorqueError = 100;

        //Algorithm
        const unsigned Siman = 1;
        const unsigned Nop = 0;

        enum OptimizationAlgorithm {
            SimulatedAnnealing = 1
        };

        enum Step{
            MinimizeTorqueErrorParameterSetDefault = 100,
            Empty = -1

        };
    }
}

#endif