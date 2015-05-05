//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_policyTemplates_h
#define ceinms_policyTemplates_h

#include "EMGDataFromFile.h"
#include "ExcitationGeneratorFromXml.h"


namespace ceinms {
    template
    class EMGDataFromFile < ExcitationGeneratorFromXml > ;
}

#endif
