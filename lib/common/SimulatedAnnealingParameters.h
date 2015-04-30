//__________________________________________________________________________
// Author(s): Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef SimulatedAnnealingParameters_h
#define SimulatedAnnealingParameters_h

struct SimulatedAnnealingParameters {
    
    unsigned noEpsilon, NS, NT, maxNoEval;
    double rt, T, epsilon;
    
};

#endif