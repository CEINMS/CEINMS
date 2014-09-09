//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef QueuesToStorageFiles_h
#define QueuesToStorageFiles_h

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "OutputQueues.h"
#include "InputConnectors.h"

class QueuesToStorageFiles {
public:
    QueuesToStorageFiles() = delete;
    QueuesToStorageFiles(const QueuesToStorageFiles&) = delete;
    QueuesToStorageFiles& operator=(const QueuesToStorageFiles&) = delete; 
    
    template <typename NMSmodelT>
    QueuesToStorageFiles(CEINMS::InputConnectors& inputConnectors, const NMSmodelT& subject, const std::vector< std::string >& valuesToWrite, const std::string& outputDir = "./Output/");
    ~QueuesToStorageFiles() = default;
    void operator()();

private:
    void initFile(const std::string& valueName, const std::vector< CEINMS::OutputConnectors::FrameType >& dataToWrite );
    void addLog(std::string valueName);
    
    std::string outputDir_;
    std::string separator_;
    std::map< std::string, std::shared_ptr<std::ostream> > outFiles_; 
    std::vector<std::string> valuesToWrite_;
    std::vector<std::string> torqueNames_; 
    std::vector<std::string> muscleNames_; 
    CEINMS::InputConnectors& inputConnectors_;
};


template <typename NMSmodelT>
QueuesToStorageFiles::QueuesToStorageFiles(CEINMS::InputConnectors& inputConnectors, const NMSmodelT& subject, const std::vector< std::string >& valuesToWrite, const std::string& outputDir)
:outputDir_(outputDir), separator_("\t"), valuesToWrite_(valuesToWrite), inputConnectors_(inputConnectors){
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

#endif