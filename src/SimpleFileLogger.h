#ifndef SimpleFileLogger_h
#define SimpleFileLogger_h

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <stdlib.h>

namespace Logger {
    enum LogID{Activations, 
                FibreLengths, 
                FibreVelocities,
                MuscleForces,
                Torques,
                Emgs,
                AdjustedEmgs};


    template <typename NMSmodelT>
    class SimpleFileLogger {


        
    public:
        SimpleFileLogger(NMSmodelT& subject);
        void addLog(LogID logID);
        void log(double time, LogID logID);
        
    private:
        void logDataVector(const std::vector<double>& data, std::ofstream& outFile);
        void initFile(const std::vector<std::string>& names, std::ofstream& outFile);
        NMSmodelT& subject_;
        
        double time_;
        std::string outputDir_;
        std::vector<boost::shared_ptr<std::ofstream> > outFiles_;    
        std::vector<LogID> fileTypes_;
    };


    template <typename NMSmodelT>
    SimpleFileLogger<NMSmodelT>::SimpleFileLogger(NMSmodelT& subject)
    :subject_(subject),
    outputDir_("./Output/") {
    
        boost::filesystem::path dir(outputDir_);
		if(!boost::filesystem::exists(dir)) {
			if(!boost::filesystem::create_directory(dir)) {
				std::cout << "Error: Cannot create the output directory " + outputDir_ << std::endl;  
				exit(EXIT_FAILURE);        
			}  
			std::cout << "Created output directory " + dir.string() << std::endl;
		}
		else std::cout << "Using " +  dir.string() + " as output directory\n";
	}
	

    template <typename NMSmodelT>
    void SimpleFileLogger<NMSmodelT>::addLog(LogID logID) {
        
        std::string filename;
        switch(logID) {
            case Activations:
                filename = "activations.csv";
                break;
            case FibreLengths:
                filename = "fiberLengths.csv";
                break;
            case FibreVelocities:
                filename = "fiberVelocities.csv";
                break;
            case MuscleForces:
                filename = "muscleTendonForces.csv";
                break;        
            case Torques:
                filename = "torques.csv";
                break; 
            case Emgs:
                filename = "emgs.csv";
                break; 
            case AdjustedEmgs:
                filename = "adjustedEmgs.csv";
                break; 
        }
        std::string outFilename =  outputDir_+filename;
        boost::shared_ptr<std::ofstream> file_ptr(new std::ofstream(outFilename.c_str()));
        outFiles_.push_back(file_ptr);
        if(!(outFiles_.back()->is_open())) {
            std::cout << "ERROR: " + filename + " cannot be opened!\n";
            exit(EXIT_FAILURE);
        }
        
        fileTypes_.push_back(logID);
        
        std::vector<std::string> names;
        if(logID == Torques)
            subject_.getDoFNames(names);
        else
            subject_.getMuscleNames(names);
        
        initFile(names, *outFiles_.back());
     
    }


    template <typename NMSmodelT>
    void SimpleFileLogger<NMSmodelT>::log(double time, LogID logID) {
    
        time_ = time;
        std::vector<double> data;
        
        switch(logID) {
            case Activations:
                subject_.getActivations(data);
                break;
            case FibreLengths:
                subject_.getFiberLengths(data);
                break;
            case FibreVelocities:
                subject_.getFiberVelocities(data);
                break;
            case MuscleForces:
                subject_.getMuscleForces(data);
                break;        
            case Torques:
                subject_.getTorques(data);
                break; 
            case Emgs:
            case AdjustedEmgs:
                subject_.getEmgs(data);
                break;    
        }

        unsigned dst = std::distance(fileTypes_.begin(), 
                                     std::find(fileTypes_.begin(), fileTypes_.end(), 
                                     logID));
        logDataVector(data, *outFiles_.at(dst));
    }
    
    

    template <typename NMSmodelT>
    void SimpleFileLogger<NMSmodelT>::logDataVector(const std::vector<double>& data, std::ofstream& outFile) {
        
        outFile << time_ << " ";
        for (unsigned i = 0; i < data.size()-1 ; ++i)
            outFile << data.at(i) << " ";
        outFile << data.back() << std::endl; 
    }


    template <typename NMSmodelT>
    void SimpleFileLogger<NMSmodelT>::initFile(const std::vector<std::string>& names, std::ofstream& outFile) {
        
        outFile << "Time ";
        for (unsigned int i = 0; i < names.size()-1 ; ++i )
            outFile << names.at(i) << " ";
        outFile << names.back() << std::endl;
    }

    
};

#endif
