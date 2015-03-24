//__________________________________________________________________________
// Author: Claudio Pizzolato, October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "Parameter.h"
#include <iostream>
#include <vector>
#include <algorithm>

template <typename NMSmodelT>
ParametersInterpreter<NMSmodelT>::ParametersInterpreter(NMSmodelT& subject, const ParameterSet& parameterSet, const std::vector<std::string>& dofsToCalibrate) :
subject_(subject), parameterSet_(parameterSet), dofNames_(dofsToCalibrate)
{
    defineParameterDetails();
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::setDofsToCalibrate(const std::vector<std::string> dofsToCalibrate) {

    dofNames_ = dofsToCalibrate;
    defineParameterDetails();
}

template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::defineParameterDetails() {

    for (ParameterSet::const_iterator it(parameterSet_.begin()); it != parameterSet_.end(); ++it) {
    //    std::cout << "Parameter " << it->getName() << std::endl;
        ParameterDetails parameterDetails;
        ParameterID parameterId = it->getID();

        ParameterAssignment assignment = it->getAssignment();
        if (assignment == Parameter::Grouped) {
            MuscleGroups muscleGroups;
            it->getMuscleGroups(muscleGroups);
            getMuscleGroupIndex(muscleGroups, parameterDetails.muscleGroups);
        }
        else
            getMuscleGroupIndex(assignment, parameterDetails.muscleGroups);

        parameterDetails.boundaryType = it->getBoundariesType();
        parameterDetails.lowerLimit = it->getLowerLimit();
        parameterDetails.upperLimit = it->getUpperLimit();
        parameterDetails.size = parameterDetails.muscleGroups.size();
        parameterDetails.name = it->getName();

        std::pair<typename ParametersMap::iterator, bool> ret;
        ret = parameters_.insert(std::pair<ParameterID, ParameterDetails>(parameterId, parameterDetails));
  //      if (ret.second == false)
   //         std::cout << "parameter " << it->getName() << " already existed - skipped\n";
    }

    noParameters_ = 0;
    for (typename ParametersMap::const_iterator it(parameters_.begin()); it != parameters_.end(); ++it)
        noParameters_ += it->second.muscleGroups.size();

  //  std::cout << "Number of Parameters " << noParameters_ << std::endl;

}



template<typename NMSmodelT>
std::vector<double> ParametersInterpreter<NMSmodelT>::getSubjectParameters() const{

    std::vector<double> x;
    for(typename ParametersMap::const_iterator it(parameters_.begin()); it != parameters_.end(); ++it) {
       
        std::vector<double> coefficients, groupedCoefficients;
		getCoefficients(it->first, coefficients);
		groupValues(it->second.muscleGroups, coefficients, groupedCoefficients);
		x.insert(x.end(), groupedCoefficients.begin(), groupedCoefficients.end());
	}
    return x;
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::setSubjectParameters(const std::vector<double>& x) {
    
    
    unsigned count = 0;
    for(typename ParametersMap::const_iterator it(parameters_.begin()); it != parameters_.end(); ++it) {
    
        unsigned noCoefficients = it->second.size;
        std::vector<double>::const_iterator startIt(x.begin()+count);
        std::vector<double>::const_iterator endIt(x.begin()+count+noCoefficients);
        std::vector<double> groupedCoefficients(startIt, endIt);
        std::vector<double> coefficients;

        getCoefficients(it->first, coefficients);
        distributeValues(it->second.muscleGroups, coefficients, groupedCoefficients);
        setCoefficients(it->first, coefficients);

        count += noCoefficients;
    
    }
    
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const {

    upperBounds.clear(); lowerBounds.clear();
    for(typename ParametersMap::const_iterator it(parameters_.begin()); it != parameters_.end(); ++it) {
        if(it->second.boundaryType == Parameter::RelativeToSubjectValue) {
            std::vector<double> coefficients, groupedCoefficients;
            getCoefficients(it->first, coefficients);
            groupValues(it->second.muscleGroups, coefficients, groupedCoefficients);
            for(std::vector<double>::const_iterator cIt(groupedCoefficients.begin()); cIt != groupedCoefficients.end(); ++cIt) {
                double upperBoundValue = *cIt * (it->second.upperLimit);
                double lowerBoundValue = *cIt * (it->second.lowerLimit);
                upperBounds.push_back(upperBoundValue);
                lowerBounds.push_back(lowerBoundValue);
            }
        }
        else {
            upperBounds.insert(upperBounds.end(), it->second.size, it->second.upperLimit);
            lowerBounds.insert(lowerBounds.end(), it->second.size, it->second.lowerLimit);
        }
    }
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::groupValues(const MuscleGroupsIdx& muscleGroupsIdx, const std::vector<double>& distributedValues, std::vector<double>& groupedValues) const{
 
    groupedValues.clear();
    for(typename MuscleGroupsIdx::const_iterator it(muscleGroupsIdx.begin()); it != muscleGroupsIdx.end(); ++it)
        groupedValues.push_back(distributedValues.at(it->front()));
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::distributeValues(const MuscleGroupsIdx& muscleGroupsIdx, std::vector<double>& distributedValues, const std::vector<double>& groupedValues) {

    for(unsigned i = 0; i < muscleGroupsIdx.size(); ++i)
        for(std::vector<unsigned>::const_iterator it(muscleGroupsIdx.at(i).begin()); it != muscleGroupsIdx.at(i).end(); ++it)
            distributedValues.at(*it) = groupedValues.at(i);
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::getMuscleGroupIndex(const MuscleGroups& muscleGroups, MuscleGroupsIdx& muscleGroupsIdx) {
    
    muscleGroupsIdx.clear();
    std::vector<std::string> muscleNames;
    subject_.getMuscleNames(muscleNames);
    for(typename MuscleGroups::const_iterator gIt(muscleGroups.begin()); gIt != muscleGroups.end(); ++gIt) {
        std::vector<unsigned> muscleIndex;
        for(std::list<std::string>::const_iterator mIt(gIt->begin()); mIt != gIt->end(); ++mIt) {
            std::vector<std::string>::iterator fId;
            fId = std::find(muscleNames.begin(), muscleNames.end(), *mIt);
			if(fId != muscleNames.end())
	            muscleIndex.push_back(std::distance(muscleNames.begin(), fId));
			else {
				std::cout << *mIt << " not found.\n";
				exit(EXIT_FAILURE);
			}
        }     
        muscleGroupsIdx.push_back(muscleIndex);
    }
}
   

//NOTE: when using global o single parameters, only the calibrating dofs are considered
template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::getMuscleGroupIndex(ParameterAssignment parameterAssignment, MuscleGroupsIdx& muscleGroupsIdx) {

    muscleGroupsIdx.clear();
    std::vector<unsigned> musclesIndexList;
    subject_.getMusclesIndexFromDofs(musclesIndexList, dofNames_);
    if(parameterAssignment == Parameter::Single) {
        for(vector<unsigned>::const_iterator it(musclesIndexList.begin()); it != musclesIndexList.end(); ++it) {
            std::vector<unsigned> m; m.push_back(*it);
            muscleGroupsIdx.push_back(m);
        }
    }
    else if(parameterAssignment == Parameter::Global) 
        muscleGroupsIdx.push_back(musclesIndexList);
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::getCoefficients(ParameterID parameterID, std::vector<double>& coefficients) const {
        
     switch(parameterID) {
            case Parameter::C1: 
                subject_.getC1Coefficients(coefficients);
                break;
            case Parameter::C2:
                subject_.getC2Coefficients(coefficients);
                break;
            case Parameter::Damping:
                break;
            case Parameter::EmDelay:
                break;
            case Parameter::MaxIsometricForce:
                break;
            case Parameter::OptimalFibreLength:
                subject_.getOptimalFibreLengths(coefficients);
                break;
            case Parameter::PennationAngle:
                break;
            case Parameter::PercentageChange:
                break;
            case Parameter::ShapeFactor:
                subject_.getShapeFactors(coefficients);
                break;
            case Parameter::StrengthCoefficient:
                subject_.getStrengthCoefficients(coefficients);
                break;
            case Parameter::TendonSlackLength:
                subject_.getTendonSlackLengths(coefficients);
                break;
        }
}


template<typename NMSmodelT>
void ParametersInterpreter<NMSmodelT>::setCoefficients(ParameterID parameterID, const std::vector<double>& coefficients) {
    
      switch(parameterID) {
            case Parameter::C1: 
                subject_.setC1Coefficients(coefficients);
                break;
            case Parameter::C2:
                subject_.setC2Coefficients(coefficients);
                break;
            case Parameter::Damping:
                break;
            case Parameter::EmDelay:
                break;
            case Parameter::MaxIsometricForce:
                break;
            case Parameter::OptimalFibreLength:
                subject_.setOptimalFibreLengths(coefficients);
                break;
            case Parameter::PennationAngle:
                break;
            case Parameter::PercentageChange:
                break;
            case Parameter::ShapeFactor:
                subject_.setShapeFactors(coefficients);
                break;
            case Parameter::StrengthCoefficient:
                subject_.setStrengthCoefficients(coefficients);
                break;
            case Parameter::TendonSlackLength:
                subject_.setTendonSlackLengths(coefficients);
                break;
        }
}