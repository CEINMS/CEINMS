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

#ifndef ceinms_Parameter_h
#define ceinms_Parameter_h

#include <string>
#include <list>

namespace ceinms {
    class Parameter {

    public:
        enum Boundaries {
            Absolute,
            RelativeToSubjectValue
        };

        enum Assignment {
            Global,
            Grouped,
            Single
        };

        enum ID {
            C1,
            C2,
            ShapeFactor,
            OptimalFibreLength,
            PennationAngle,
            TendonSlackLength,
            MaxIsometricForce,
            MaxContractionVelocity,
            StrengthCoefficient,
            EmDelay,
            Damping,
            PercentageChange,
            NotFound
        };

        typedef std::list<std::string> MuscleNames;
        typedef std::list<MuscleNames> MuscleGroups;
        using Set = std::list < Parameter > ;

        void setName(const std::string& name);
        std::string getName() const { return name_; }
        void setAssignment(Assignment assignment);
        Assignment getAssignment() const { return assignment_; }
        Boundaries getBoundariesType() const { return boundariesType_; }
        void getMuscleGroups(MuscleGroups& muscleGroups) const { muscleGroups = muscleGroups_; }
        bool hasMuscleGroups() const { return !muscleGroups_.empty(); }
        ID getID() const;
        void pushMuscleGroup(const MuscleNames& group);
        void setBoundariesType(Boundaries boundaries);
        void setLowerAndUpperLimits(double lowerLimit, double upperLimit);
        double getLowerLimit() const { return lowerLimit_; }
        double getUpperLimit() const { return upperLimit_; }
        friend std::ostream& operator<< (std::ostream& output, const Parameter& b);
    private:
        std::string name_;
        MuscleGroups muscleGroups_;
        Boundaries boundariesType_;
        Assignment assignment_;
        double lowerLimit_;
        double upperLimit_;
    };
}


#endif
