//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "EMGgeneratorFromXml.h"
#include "emgGenerator.hxx"
#include <vector>
using std::vector;
#include <string>
using std::string;

#include <algorithm>
using std::find;
#include <iostream>
#include <stdlib.h>
#include <boost/concept_check.hpp>



EMGgeneratorFromXml::EMGgeneratorFromXml(const std::string& xmlFilename) {
    
    try {
    std::auto_ptr<emgGenerator> generatorPtr(emgGenerator_(xmlFilename.c_str()));
    
    emgGenerator::experimental_type& experimentalEmgs(generatorPtr->experimental());
    EmgListType::emg_sequence& experimentalEmgList(experimentalEmgs.emg());
    for(EmgListType::emg_sequence::iterator it(experimentalEmgList.begin()); it != experimentalEmgList.end(); ++it) 
        fromMusclesNames_.push_back(it->id()->data());
    
    emgGenerator::output_type& outputEmgs(generatorPtr->output());
    for(StringListType::iterator it(outputEmgs.begin()); it != outputEmgs.end(); ++it) 
        toMusclesNames_.push_back(*it);
    
    functors_.assign(toMusclesNames_.size(), new Nil<double>);
    argumentIndex_.resize(toMusclesNames_.size());
    
    //find all the matches and set the Assign functor
    for(vector<string>::iterator it(fromMusclesNames_.begin()); it != fromMusclesNames_.end(); ++it) {
        vector<string>::iterator fIt = std::find(toMusclesNames_.begin(), toMusclesNames_.end(), *it);
        if (fIt != toMusclesNames_.end()) {
            unsigned dst = std::distance(toMusclesNames_.begin(), fIt);
            functors_.at(dst) = new Assign<double, double>;
            argumentIndex_.at(dst) = vector<unsigned>(1, std::distance(fromMusclesNames_.begin(), it));
        }   
    }
   
    emgGenerator::mapping_type& mappedEmgs(generatorPtr->mapping());
    EmgListType::emg_sequence& mappedEmgList(mappedEmgs.emg());
    for(EmgListType::emg_sequence::iterator it(mappedEmgList.begin()); it != mappedEmgList.end(); ++it) {
        vector<string>::iterator fIt = std::find(toMusclesNames_.begin(), toMusclesNames_.end(),it->id()->data());
        if(fIt != toMusclesNames_.end()) {
            unsigned dst = std::distance(toMusclesNames_.begin(), fIt);
            setEmgMappingOperation(dst, it->operation()->data());
            vector<string> muscleNames(it->begin(), it->end());
            setEmgMappingArguments(dst, muscleNames);
        }
    }
    
    noFromMuscles_ = fromMusclesNames_.size();
    noToMuscles_   = toMusclesNames_.size();
    
    } catch (const xml_schema::exception& e) {
        std::cout << e << std::endl;
        exit(EXIT_FAILURE);
    }
    /*
    std::cout << "Experimental EMGs\n";
    for(vector<string>::const_iterator it(fromMusclesNames_.begin()); it != fromMusclesNames_.end(); ++it)
        std::cout << *it << " ";
    std::cout <<std::endl;

    std::cout << "Output EMGs\n";
    for(vector<string>::const_iterator it(toMusclesNames_.begin()); it != toMusclesNames_.end(); ++it)
        std::cout << *it << " ";
    std::cout <<std::endl;
    */
    
}


void EMGgeneratorFromXml::setEmgMappingOperation(unsigned int index, const string& operation) {

    if(operation == "assign")
        functors_.at(index) = new Assign<double, double>;
    else if (operation == "mean")
        functors_.at(index) = new Mean<double, vector<double> >;
    else if(operation == "sum")
        functors_.at(index) = new Sum<double, vector<double> >;
    else
        functors_.at(index) = new Nil<double>;
}


void EMGgeneratorFromXml::setEmgMappingArguments(unsigned int index, const vector< string >& muscleNames) {

    for(vector<string>::const_iterator it(muscleNames.begin()); it != muscleNames.end(); ++it) {
        vector<string>::iterator fIt = std::find(fromMusclesNames_.begin(), fromMusclesNames_.end(), *it);
        if (fIt != fromMusclesNames_.end()) {
            unsigned dst = std::distance(fromMusclesNames_.begin(), fIt);
            argumentIndex_.at(index).push_back(dst);
        } else {
            std::cout << *it << " not found" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}


void EMGgeneratorFromXml::convert(const std::vector< double >& fromEMG, std::vector< double >& toEMG) const {

    if (fromEMG.size() != noFromMuscles_) {
        std::cout << "I need " << noFromMuscles_ << " muscles, but I got " << fromEMG.size() << " muscles!\n";
        exit(EXIT_FAILURE); 
    }
  
    if (toEMG.size() != noToMuscles_) {
        std::cout << "I need " << noToMuscles_ << " muscles, but I got " << toEMG.size() << " muscles!\n";
        exit(EXIT_FAILURE); 
    }
  
    toEMG.clear();
    toEMG.resize(noToMuscles_);
    for(unsigned i = 0; i < noToMuscles_; ++i) {
        double result;
        if(argumentIndex_.at(i).size() == 1) {
            (*functors_.at(i))(&result, fromEMG.at(argumentIndex_.at(i).front())); 
        } else if(!argumentIndex_.at(i).empty()) {
            vector<double> arguments;
            for(vector<unsigned>::const_iterator it(argumentIndex_.at(i).begin()); it != argumentIndex_.at(i).end(); ++it)
                arguments.push_back(fromEMG.at(*it));
            (*functors_.at(i))(&result, arguments);
        }
         else  (*functors_.at(i))(&result);
        toEMG.at(i) = result;    
    }
}






