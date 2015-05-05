//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_OutputConnectors_h
#define ceinms_OutputConnectors_h

#include "Queue.h"
#include "Latch.h"
#include "QueueData.h"

#include <map>
#include <string>
#include <vector>

namespace ceinms {
    class OutputConnectors {
    public:
        OutputConnectors();
        virtual ~OutputConnectors();
        OutputConnectors(const OutputConnectors&) = delete;
        using DataType = std::vector < double > ;
        using FrameType = QueueData < DataType > ;

        const double TimePlaceholderForEndOfData = std::numeric_limits<double>::infinity();
        std::map< std::string, Concurrency::Queue< FrameType >* > logQueues;
        Concurrency::Latch doneWithExecution;
    };
}

#endif