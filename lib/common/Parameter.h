//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef Parameter_h
#define Parameter_h

#include <string>
#include <list>

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
        StrengthCoefficient,
        EmDelay,
        Damping,
        PercentageChange,
        NotFound
    };
    
    typedef std::list<std::string> MuscleNames;
    typedef std::list<MuscleNames> MuscleGroups;
    

    void setName(const std::string& name);
    std::string getName() const { return name_;}
    void setAssignment(Assignment assignment);
    Assignment getAssignment() const {return assignment_;}
    Boundaries getBoundariesType() const {return boundariesType_;}
    void getMuscleGroups(MuscleGroups& muscleGroups) const { muscleGroups = muscleGroups_;}
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



#endif