//__________________________________________________________________________
// Author: Claudio Pizzolato, October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ParametersInterpreter_h
#define ParametersInterpreter_h

#include "Parameter.h"

#include <list>
#include <vector>
#include <string>
#include <map>
//#include <boost/concept_check.hpp>

//should try to temove this template.. maybe in future..
template <typename NMSmodelT>
class ParametersInterpreter  {
    
public:
    using ParameterSet = Parameter::Set;
    ParametersInterpreter(NMSmodelT& subject, const ParameterSet& parameterSet, const std::vector<std::string>& dofsToCalibrate);
    size_t getNoParameters() const { return noParameters_; }
    std::vector<double> getSubjectParameters() const;
    void setSubjectParameters(const std::vector<double>& x);
    void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const;
    void setDofsToCalibrate(const std::vector<std::string> dofsToCalibrate);
private:

    //modify 
    typedef typename Parameter::MuscleGroups MuscleGroups;
    typedef std::vector< std::vector<unsigned> > MuscleGroupsIdx;
    typedef typename Parameter::ID ParameterID;
    typedef typename Parameter::Assignment ParameterAssignment;
    typedef typename Parameter::Boundaries ParameterBoundaries;    

    struct ParameterDetails {
        MuscleGroupsIdx muscleGroups;
        ParameterBoundaries boundaryType;
        std::string name;
        double lowerLimit;
        double upperLimit;
        unsigned size;
    };
    

    typedef std::map<ParameterID, ParameterDetails> ParametersMap;
    void defineParameterDetails();
    
    void groupValues(const MuscleGroupsIdx& muscleGroupIdx, const std::vector<double>& distributedValues, std::vector<double>& groupedValues) const;
    void distributeValues(const MuscleGroupsIdx& muscleGroupIdx, std::vector<double>& distributedValues, const std::vector<double>& groupedValues);
    void getMuscleGroupIndex(const MuscleGroups& muscleGroups, MuscleGroupsIdx& muscleGroupsIdx);
    void getMuscleGroupIndex(ParameterAssignment parameterAssignment, MuscleGroupsIdx& muscleGroupsIdx);
    void getCoefficients(ParameterID parameterID, std::vector<double>& coefficients) const;
    void setCoefficients(ParameterID parameterID, const std::vector<double>& coefficients);
    NMSmodelT& subject_;
    ParameterSet parameterSet_;
    ParametersMap parameters_;
    std::vector<std::string> dofNames_;
    size_t noParameters_;
    
};

#include "ParametersInterpreter.cpp"

#endif