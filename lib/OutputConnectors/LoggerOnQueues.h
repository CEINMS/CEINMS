//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_LoggerOnQueues_h
#define ceinms_LoggerOnQueues_h

#include <vector>
#include <string>
#include "OutputConnectors.h"

namespace ceinms {
    class LoggerOnQueues {
    public:
        LoggerOnQueues() = delete;
        LoggerOnQueues(const LoggerOnQueues&) = delete;
        LoggerOnQueues& operator=(const LoggerOnQueues&) = delete;
        LoggerOnQueues(OutputConnectors& outputConnectors, const std::vector< std::string > & valuesToLog);
        void log(double time, const std::vector< double >& dataToLog, const std::string& queuID);

    private:
        OutputConnectors& outputConnectors_;

    };
}

#endif