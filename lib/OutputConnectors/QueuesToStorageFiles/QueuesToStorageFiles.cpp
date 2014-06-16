#include "QueuesToStorageFiles.h"
#include "InputQueues.h"

void QueuesToStorageFiles::addLog(std::string valueName)
{
    std::string filename = valueName + ".sto"; 
    std::string outFilename =  outputDir_ + filename;
  
    std::shared_ptr<std::ofstream> file_ptr(new std::ofstream(outFilename.c_str()));
    if (!(file_ptr->is_open()))  {
        std::cout << "ERROR: " + filename + " cannot be opened!\n";
        exit(EXIT_FAILURE);
    }
  
    outFiles_.insert(std::make_pair(valueName, file_ptr)); 
  
}

void QueuesToStorageFiles::operator()() {
  
    for (auto& it:valuesToWrite_) {

        auto q = CEINMS::OutputConnectors::logQueues.find(it);
        if (q == CEINMS::OutputConnectors::logQueues.end() ) {
            std::cout << it << " queue was not found\n";
            exit(EXIT_FAILURE);
        } 
        else {
            q->second->subscribe(); 
            addLog(it); 
        }
    }
    
    CEINMS::InputConnectors::doneWithSubscription.wait();
    
    CEINMS::OutputConnectors::doneWithExecution.wait();
    
    
      for (auto& currentValue:valuesToWrite_) {
        
       auto currentQueue = CEINMS::OutputConnectors::logQueues.find(currentValue);
       if (currentQueue == CEINMS::OutputConnectors::logQueues.end() ) {
           std::cout << currentValue << " queue was not found\n";
           exit(EXIT_FAILURE);
       } 
      
       
    
       CEINMS::OutputConnectors::FrameType currentFrame;
       std::vector< CEINMS::OutputConnectors::DataType > dataToWrite;
       
       currentFrame = currentQueue->second->pop(); 
       while (currentFrame.time != CEINMS::OutputConnectors::TimePlaceholderForEndOfData) {
         dataToWrite.push_back(currentFrame.data);
         currentFrame = currentQueue->second->pop();
       }

          
       initFile(currentValue, dataToWrite);
       auto q = outFiles_.find(currentValue);
       auto outFile = (q->second);
       for(auto& dataIt: dataToWrite) {
           for(auto& sampleIt:dataIt)
             *outFile << sampleIt << separator_; 
           *outFile << std::endl;     
       }     
   
        
   }
   
   
      
}

 
void QueuesToStorageFiles::initFile(const std::string& valueName, const std::vector< CEINMS::OutputConnectors::DataType >& dataToWrite ) 
{

    auto q = outFiles_.find(valueName);
    if (q == outFiles_.end() ) {
        std::cout << valueName << " file was not found\n";
        exit(EXIT_FAILURE);
    } 
    
    auto outFile = (q->second);
    *outFile << "CEINMS output" << std::endl;
    *outFile << "datacolumns " << dataToWrite.back().size() <<std::endl;
    *outFile << "datarows " << dataToWrite.size() <<std::endl;
    *outFile << "endheader" << std::endl;

    
    *outFile << "time" + separator_; 
    if (valueName == "Torques")
        for(auto& it: torqueNames_)
            *outFile << it << separator_;
    else 
        for(auto& it: muscleNames_)
            *outFile << it << separator_;  
    *outFile << std::endl; 
}

