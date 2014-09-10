//__________________________________________________________________________
// Author(s): Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef CeinmsLogger_h
#define CeinmsLogger_h


namespace Logger {
    enum LogID{Activations,
                FibreLengths,
                FibreVelocities,
                MuscleForces,
                MtusStiffness,
                Torques,
				dofsStiffness,
                Emgs,
                AdjustedEmgs};
}

#endif
