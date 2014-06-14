//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef DataToStorageFiles_h
#define DataToStorageFiles_h

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "../OutputQueues.h"
#include "../InputConnectors/InputQueues.h"

class DataToStorageFiles {
public:
    DataToStorageFiles() = delete;
    template <typename NMSmodelT>
    DataToStorageFiles(const NMSmodelT& subject, const std::vector< std::string >& valuesToWrite, const std::string& outputDir ="./Output/");
    ~DataToStorageFiles();
    void operator()();

private:
    void initFile(const std::string& valueName, const std::vector< std::vector< double > >& dataToWrite );
    void addLog(std::string valueName);
    
    double time_;
    std::string outputDir_;
    std::string separator_;
    std::map< std::string, std::shared_ptr<std::ostream> > outFiles_; 
    std::vector<std::string> valuesToWrite_;
    std::vector<std::string> torqueNames_; 
    std::vector<std::string> muscleNames_; 
};


template <typename NMSmodelT>
DataToStorageFiles::DataToStorageFiles(const NMSmodelT& subject, const std::vector< std::string >& valuesToWrite, const std::string& outputDir)
:outputDir_(outputDir), separator_("\t"), valuesToWrite_(valuesToWrite){
    outputDir_ += "/";
    boost::filesystem::path dir(outputDir_);
    if(!boost::filesystem::exists(dir)) {
        if(!boost::filesystem::create_directory(dir)) {
            std::cout << "Error: Cannot create the output directory " + outputDir_ << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Created output directory " + dir.string() << std::endl;
    } else std::cout << "Using " +  dir.string() + " as output directory\n";
    
    subject.getDoFNames(torqueNames_);
    subject.getMuscleNames(muscleNames_);

}


void DataToStorageFiles::addLog(std::string valueName)
{
  std::string filename = valueName + ".sto"; 
  std::string outFilename =  outputDir_+filename;
  
  std::shared_ptr<std::ofstream> file_ptr(new std::ofstream(outFilename.c_str()));
  if (!(file_ptr->is_open()))  {
      std::cout << "ERROR: " + filename + " cannot be opened!\n";
      exit(EXIT_FAILURE);
  }
  
  outFiles_.insert(std::make_pair(valueName, file_ptr)); 
  
}

void DataToStorageFiles::operator()() {
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
      
       std::vector< std::vector< double > > dataToWrite;
    
       CEINMS::OutputConnectors::FrameType currentFrame;
       double TimeLimit = std::numeric_limits<double>::infinity();
       currentFrame = currentQueue->second->pop(); 
       while (currentFrame.time != TimeLimit) {
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

 
void DataToStorageFiles::initFile(const std::string& valueName, const std::vector< std::vector< double > >& dataToWrite ) {
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


DataToStorageFiles::~DataToStorageFiles()
{ 
  
 
}



#endif