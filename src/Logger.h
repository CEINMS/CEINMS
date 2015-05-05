//__________________________________________________________________________
// Author(s): Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_Logger_h
#define ceinms_Logger_h

namespace ceinms {
    namespace Logger {
        enum LogID{
            Activations,
            FibreLengths,
            NormFibreLengths,
            FibreVelocities,
            NormFibreVelocities,
            PennationAngles,
            MuscleForces,
            Torques,
            Emgs,
            AdjustedEmgs
        };
    }
}
#endif
