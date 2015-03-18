//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "LoggerOnQueues.h"
#include "OutputConnectors.h"
#include <vector>
#include <string>
#include <iostream>
using namespace CEINMS;

LoggerOnQueues::LoggerOnQueues(OutputConnectors& outputConnector, const std::vector< std::string >& valuesToLog):
outputConnectors_(outputConnector)
{
  
    for (auto& it: valuesToLog) 
        outputConnectors_.logQueues.insert(std::make_pair(it, new CEINMS::Concurrency::Queue< CEINMS::OutputConnectors::FrameType>) ); 

  
}

void LoggerOnQueues::log(double time, const CEINMS::OutputConnectors::DataType& dataToLog, const std::string& valueToLog)
{
  
    OutputConnectors::FrameType dataToPush;
    dataToPush.data = dataToLog; 
    dataToPush.time = time; 
    
    auto currentQueue = outputConnectors_.logQueues.find(valueToLog);

    if (currentQueue == outputConnectors_.logQueues.end()) {
        std::cout << "queue for " << valueToLog << " was not found\n";
        exit(EXIT_FAILURE);
    }
    else 
        currentQueue->second->push(dataToPush);   

  
}

