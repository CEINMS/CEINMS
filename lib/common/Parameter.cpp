//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "Parameter.h"
#include <iostream>
#include <string>
using std::string;

#include <boost/algorithm/string.hpp>



void Parameter::setName(const string& name) {

    name_ = name;
}


Parameter::ID Parameter::getID() const{

    ID id;
    if     (boost::iequals(name_, "c1"))                     id = C1;
    else if(boost::iequals(name_, "c2"))                     id = C2;
    else if(boost::iequals(name_, "shapeFactor"))            id = ShapeFactor;
    else if(boost::iequals(name_, "optimalFibreLength"))     id = OptimalFibreLength;
    else if(boost::iequals(name_, "pennationAngle"))         id = PennationAngle;
    else if(boost::iequals(name_, "tendonSlackLength"))      id = TendonSlackLength;
    else if(boost::iequals(name_, "maxContractionVelocity")) id = MaxContractionVelocity;
    else if(boost::iequals(name_, "maxIsometricForce"))      id = MaxIsometricForce;
    else if(boost::iequals(name_, "strengthCoefficient"))    id = StrengthCoefficient;
    else if(boost::iequals(name_, "emDelay"))                id = EmDelay;
    else if(boost::iequals(name_, "damping"))                id = Damping;
    else if(boost::iequals(name_, "percentageChange"))       id = PercentageChange;
    else                                                     id = NotFound;
    
    return id;

}


void Parameter::setAssignment(Parameter::Assignment assignment) {

    assignment_ = assignment;
}


void Parameter::pushMuscleGroup(const Parameter::MuscleNames& group) {

    muscleGroups_.push_back(group);
}

void Parameter::setBoundariesType(Parameter::Boundaries boundaries) {

    boundariesType_ = boundaries;
}


void Parameter::setLowerAndUpperLimits(double lowerLimit, double upperLimit) {

    lowerLimit_ = lowerLimit;
    upperLimit_ = upperLimit;
}


std::ostream& operator<< (std::ostream& output, const Parameter& rhs) {
    
    output << " ---- Parameter: " << rhs.name_ << std::endl;
    switch(rhs.assignment_) {
        case Parameter::Global:
            output << " ---- Global\n";
            break;
        case Parameter::Grouped:
            output << " ---- Grouped:\n";
            for(Parameter::MuscleGroups::const_iterator mgIt = rhs.muscleGroups_.begin(); mgIt != rhs.muscleGroups_.end(); ++mgIt) {
                output << " ----- Group: ";
                for(Parameter::MuscleNames::const_iterator mIt = mgIt->begin(); mIt != mgIt->end(); ++mIt)
                    output << *mIt << " ";
                output << std::endl;
            }        
            break;
        case Parameter::Single:
            output << " ---- Single\n";
            break;
        default:
            output << " ---- Not Found\n";
            break;
        
    }
    switch(rhs.boundariesType_) {
        case (Parameter::Absolute):
            output << " ----- Absolute Range ";
            break;
        case (Parameter::RelativeToSubjectValue):
            output << " ----- Relative Range ";
            break;
        default:
            output << " ----- Not Found\n";
    }
    output << rhs.lowerLimit_ << " " << rhs.upperLimit_ << std::endl;
    return output;
}