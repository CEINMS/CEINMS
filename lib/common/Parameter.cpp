/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): C. Pizzolato                                                    *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "Parameter.h"
#include <iostream>
#include <string>
using std::string;

#include <boost/algorithm/string.hpp>


namespace ceinms {
    void Parameter::setName(const string& name) {

        name_ = name;
    }


    Parameter::ID Parameter::getID() const{

        ID id;
        if (boost::iequals(name_, "c1"))                     id = C1;
        else if (boost::iequals(name_, "c2"))                     id = C2;
        else if (boost::iequals(name_, "shapeFactor"))            id = ShapeFactor;
        else if (boost::iequals(name_, "optimalFibreLength"))     id = OptimalFibreLength;
        else if (boost::iequals(name_, "pennationAngle"))         id = PennationAngle;
        else if (boost::iequals(name_, "tendonSlackLength"))      id = TendonSlackLength;
        else if (boost::iequals(name_, "maxContractionVelocity")) id = MaxContractionVelocity;
        else if (boost::iequals(name_, "maxIsometricForce"))      id = MaxIsometricForce;
        else if (boost::iequals(name_, "strengthCoefficient"))    id = StrengthCoefficient;
        else if (boost::iequals(name_, "emDelay"))                id = EmDelay;
        else if (boost::iequals(name_, "damping"))                id = Damping;
        else if (boost::iequals(name_, "percentageChange"))       id = PercentageChange;
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
        switch (rhs.assignment_) {
        case Parameter::Global:
            output << " ---- Global\n";
            break;
        case Parameter::Grouped:
            output << " ---- Grouped:\n";
            for (Parameter::MuscleGroups::const_iterator mgIt = rhs.muscleGroups_.begin(); mgIt != rhs.muscleGroups_.end(); ++mgIt) {
                output << " ----- Group: ";
                for (Parameter::MuscleNames::const_iterator mIt = mgIt->begin(); mIt != mgIt->end(); ++mIt)
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
        switch (rhs.boundariesType_) {
        case (Parameter::Absolute) :
            output << " ----- Absolute Range ";
            break;
        case (Parameter::RelativeToSubjectValue) :
            output << " ----- Relative Range ";
            break;
        default:
            output << " ----- Not Found\n";
        }
        output << rhs.lowerLimit_ << " " << rhs.upperLimit_ << std::endl;
        return output;
    }
}
