//__________________________________________________________________________
// Author(s): Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_SimulatedAnnealingParameters_h
#define ceinms_SimulatedAnnealingParameters_h

namespace ceinms {
    struct SimulatedAnnealingParameters {

        unsigned noEpsilon, NS, NT, maxNoEval;
        double rt, T, epsilon;

    };
}
#endif