#ifndef CalibrationXmlReader_h
#define CalibrationXmlReader_h

#include <string>
#include <vector>
#include <map>
#include <list>
#include <string>
#include "NMSmodelConfig.h"
#include "CalibrationCfg.h"
#include "calibration.hxx"


struct SimulatedAnnealingParameters {
    
    int noEpsilon, NS, NT, maxNoEval;
    double rt, T, epsilon;
    
};

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
    
    typedef std::list<std::string> MuscleNames;
    typedef std::list<MuscleNames> MuscleGroups;
    

    void setName(const std::string& name);
    void setAssignment(Assignment assignment);
    void pushMuscleGroup(const MuscleNames& group);
    void setBoundariesType(Boundaries boundaries);
    void setLowerAndUpperLimits(double lowerLimit, double upperLimit);
    friend std::ostream& operator<< (std::ostream& output, const Parameter& b);
private:
    std::string name_;
    MuscleGroups muscleGroups_;
    Boundaries boundariesType_;
    Assignment assignment_;
    double lowerLimit_;
    double upperLimit_;
};

typedef std::list<Parameter> ParameterSet ;

class CalibrationStep {
     
public:
    void setObjectiveFunction(unsigned objectiveFunction);    
    void setParametersPolicy(unsigned parametersPolicy);    
    void setComputationMode(unsigned computationMode);    
    CalibrationCfg::Step getStepCfg() const;
    
    void pushDofName(std::string& dofName);
    void getDofNames(std::list<std::string>& dofNames);
    
    void pushParameter(const Parameter& parameter);
    bool getParameterSet(ParameterSet& parameterSet) const;
    
    friend std::ostream& operator<< (std::ostream& output, const CalibrationStep& b);
    
private:
    unsigned objectiveFunction_;
    unsigned parametersPolicy_;
    unsigned computationMode_;
    std::list<std::string> dofNames_;
    ParameterSet parameterSet_;
};



class CalibrationXmlReader {
    
public:
    CalibrationXmlReader(const std::string& filename);
    NMSModelCfg::RunMode getNMSmodelRunMode() const;
    CalibrationCfg::OptimizationAlgorithm getOptimizationAlgorithm() const;
    void getOptimizationAlgorithmParameters(SimulatedAnnealingParameters& parameters) const;  //overload this function when adding new opt algorithms
    bool popNextCalibrationStep(CalibrationCfg::Step& stepCfg, ParameterSet& parameterSet); 
   
    friend std::ostream& operator<< (std::ostream& output, const CalibrationXmlReader& b);
    
private:
    void readXml();
    void readNMSmodelCfg();
    void readOptimizationAlgorithmCfg();
    void readCalibrationStepsCfg();
    void readStep(StepType& stepType, CalibrationStep& calibrationStep);
    void readParameter(ParameterType& parameterType, Parameter& parameter);
    void readCalibrationTrialList();
    unsigned runMode_;
    unsigned optimizationAlgorithm_;
    std::list<CalibrationStep> calibrationSteps_;
    std::list<std::string> calibrationTrials_;
    std::auto_ptr<CalibrationType> calibrationPointer_;    
    SimulatedAnnealingParameters simanParameters_;
    
};

#endif