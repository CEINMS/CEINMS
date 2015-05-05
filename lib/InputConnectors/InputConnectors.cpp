//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "InputConnectors.h"
#include <vector>

namespace ceinms {

    InputConnectors::InputConnectors() :
        globalTimeLimit(std::numeric_limits<double>::infinity()), externalTorquesAvailable(false) { }


    InputConnectors::~InputConnectors() {
    
        if (!queueMomentArms.empty()) {
            for (auto it : queueMomentArms)
                delete it;
        }
    }

    //:TODO: I have no idea about this globalTimeLimit.... WHY... AND WHY SO HIDDEN.. come on
    //    float globalTimeLimit = 100.;

}
