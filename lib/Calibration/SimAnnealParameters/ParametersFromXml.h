//__________________________________________________________________________
// Author: Claudio Pizzolato, October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ParametersFromXml_h
#define ParametersFromXml_h

#include "ParametersToOptimize.h"
#include "Parameter.h"

#include <list>
#include <vector>
#include <string>
#include <map>
#include <boost/concept_check.hpp>

template <typename NMSmodelT>
class ParametersFromXml : public ParametersToOptimize  {
    
public:
    typedef std::list<Parameter> ParameterSet ;
    ParametersFromXml(NMSmodelT& subject, std::vector<std::string>& dofToCalibrate, const ParameterSet& );
    int getNoParameters() { return noParameters_; }
    void getStartingVectorParameters(std::vector<double>& x);
    void setVectorParameters(const std::vector<double>& x);
    void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds);
    void test();
    
private:

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
    void groupValues(const MuscleGroupsIdx& muscleGroupIdx, const std::vector<double>& distributedValues, std::vector<double>& groupedValues);
    void distributeValues(const MuscleGroupsIdx& muscleGroupIdx, std::vector<double>& distributedValues, const std::vector<double>& groupedValues);
    void getMuscleGroupIndex(const MuscleGroups& muscleGroups, MuscleGroupsIdx& muscleGroupsIdx);
    void getMuscleGroupIndex(ParameterAssignment parameterAssignment, MuscleGroupsIdx& muscleGroupsIdx);
    void getCoefficients(ParameterID parameterID, std::vector<double>& coefficients);
    void setCoefficients(ParameterID parameterID, const std::vector<double>& coefficients);
    NMSmodelT& subject_;
    ParametersMap parameters_;
    std::vector<std::string> dofNames_;
    unsigned noParameters_;
    
};

#include "ParametersFromXml.cpp"

#endif