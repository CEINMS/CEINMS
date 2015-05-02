//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

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
#include "SimulatedAnnealingParameters.h"
#include "Parameter.h"


typedef std::list<Parameter> ParameterSet ;

class CalibrationStep {
     
public:
    void setObjectiveFunction(unsigned objectiveFunction);    
    void setParametersPolicy(unsigned parametersPolicy);    
    CalibrationCfg::Step getStepCfg() const;
    
    void pushDofName(std::string& dofName);
    void getDofNames(std::list<std::string>& dofNames) const;
    void getDofNames(std::vector<std::string>& dofNames) const;
    
    void pushParameter(const Parameter& parameter);
    void getParameterSet(ParameterSet& parameterSet) const;
    
    friend std::ostream& operator<< (std::ostream& output, const CalibrationStep& b);
    
private:
    unsigned objectiveFunction_;
    std::list<std::string> dofNames_;
    ParameterSet parameterSet_;
};



class CalibrationXmlReader {
    
public:
    CalibrationXmlReader(const std::string& filename);
    NMSModelCfg::RunMode getNMSmodelRunMode() const;
    CalibrationCfg::OptimizationAlgorithm getOptimizationAlgorithm() const;
    void getOptimizationAlgorithmParameters(SimulatedAnnealingParameters& parameters) const;  //overload this function when adding new opt algorithms
    bool popNextCalibrationStep(CalibrationStep& calibrationStep); 
    void getCalibrationTrials(std::list<std::string>& trials) const;
    double getTolerance() const { return tolerance_; };
    friend std::ostream& operator<< (std::ostream& output, const CalibrationXmlReader& b);
    
private:
    void readXml();
    void readNMSmodelCfg();
    void readOptimizationAlgorithmCfg();
    void readCalibrationStepsCfg();
    void readStep(CalibrationXsd::StepType& stepType, CalibrationStep& calibrationStep);
    void readParameter(CalibrationXsd::ParameterType& parameterType, Parameter& parameter);
	void readCalibrationTrialList();
    unsigned runMode_;
    unsigned optimizationAlgorithm_;
	std::string trialsDirectory_;
    std::list<CalibrationStep> calibrationSteps_;
    std::list<std::string> calibrationTrials_;
    std::auto_ptr<CalibrationXsd::CalibrationType> calibrationPointer_;    
    SimulatedAnnealingParameters simanParameters_;
    std::string filepath_;
    double tolerance_;
};

#endif