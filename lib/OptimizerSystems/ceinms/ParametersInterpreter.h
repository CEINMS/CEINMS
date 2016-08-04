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

#ifndef ceinms_ParametersInterpreter_h
#define ceinms_ParametersInterpreter_h

#include "ceinms/Parameter.h"

#include <list>
#include <vector>
#include <string>
#include <map>

namespace ceinms {
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
}

#include "ParametersInterpreter.cpp"
#endif
