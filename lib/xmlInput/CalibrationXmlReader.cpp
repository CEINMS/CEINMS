//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "CalibrationXmlReader.h"

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <ostream>
#include "calibration.hxx"
#include "calibration-schema.hxx"
#include "validation.h"
using namespace CalibrationXsd;

CalibrationXmlReader::CalibrationXmlReader(const string& filename)
:runMode_(0), optimizationAlgorithm_(0) {

    try {
        std::auto_ptr<CalibrationType> calibrationPointer(parseAndValidate<CalibrationType>(filename, calibration_schema, sizeof(calibration_schema)));
        calibrationPointer_ = calibrationPointer;
    }  
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Xml reader constructor\n";
    readXml();
}


void CalibrationXmlReader::readXml() {

    
    readNMSmodelCfg();
    readOptimizationAlgorithmCfg();
    readCalibrationStepsCfg();    
	readCalibrationTrialsDirectory();
    readCalibrationTrialList();
}


void CalibrationXmlReader::readNMSmodelCfg() {

    std::cout << "readNMSmodelCfg\n";
    try {
        CalibrationType::NMSmodel_type& myModel(calibrationPointer_->NMSmodel());
        CalibrationType::NMSmodel_type::activation_type& myActivation(myModel.activation());
        CalibrationType::NMSmodel_type::activation_type::exponential_optional& myExpOpt(myActivation.exponential());
        CalibrationType::NMSmodel_type::activation_type::piecewise_optional& myPWOpt(myActivation.piecewise());

        if(myExpOpt.present())
            runMode_ += NMSModelCfg::ExponentialActivation;
        else if(myPWOpt.present())
            runMode_ += NMSModelCfg::PiecewiseActivation;
        else {
            cout << "invalid XML: ActivationType not found\n";
            exit(EXIT_FAILURE);
        }
    
        CalibrationType::NMSmodel_type::tendon_type& myTendon(myModel.tendon());
        CalibrationType::NMSmodel_type::tendon_type::stiff_optional& myStiffOpt(myTendon.stiff());
        CalibrationType::NMSmodel_type::tendon_type::elastic_optional& myElsaticOpt(myTendon.elastic());
        CalibrationType::NMSmodel_type::tendon_type::elasticBiSec_optional& myElsaticBiSecOpt(myTendon.elasticBiSec());
    
        if(myStiffOpt.present())
            runMode_ += NMSModelCfg::StiffTendon;
        else if(myElsaticOpt.present())
            runMode_ += NMSModelCfg::ElasticTendon;
        else if(myElsaticBiSecOpt.present())
            runMode_ += NMSModelCfg::ElasticTendonBiSec;
        else {
            cout << "invalid XML: TendonType not found\n";
            exit(EXIT_FAILURE);
        }
        
        CalibrationType::NMSmodel_type::type_type& myType(myModel.type());
        runMode_ += NMSModelCfg::OpenLoop;
        
        runMode_ += NMSModelCfg::Offline;
        
    } catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
    
}


void CalibrationXmlReader::readOptimizationAlgorithmCfg(){

    CalibrationType::algorithm_type& myAlgorithm(calibrationPointer_->algorithm());
    
    // siman is the only choice
    optimizationAlgorithm_ += CalibrationCfg::Siman;
    
    CalibrationType::algorithm_type::simulatedAnnealing_type& mySiman(myAlgorithm.simulatedAnnealing());
    simanParameters_.noEpsilon = mySiman.noEpsilon();
    simanParameters_.rt        = mySiman.rt();
    simanParameters_.T         = mySiman.T();
    simanParameters_.NS        = mySiman.NS();
    simanParameters_.NT        = mySiman.NT();
    simanParameters_.epsilon   = mySiman.epsilon();
    simanParameters_.maxNoEval = mySiman.maxNoEval();   
    
}


void CalibrationXmlReader::readCalibrationStepsCfg() {
    
    CalibrationType::calibrationSteps_type& myCalibrationSteps(calibrationPointer_->calibrationSteps());
    CalibrationType::calibrationSteps_type::step_sequence& stepSequence(myCalibrationSteps.step());
    CalibrationType::calibrationSteps_type::step_iterator stepIt = stepSequence.begin();
    for(stepIt; stepIt != stepSequence.end(); ++stepIt) {
        CalibrationStep calibrationStep;
        readStep(*stepIt, calibrationStep);
        calibrationSteps_.push_back(calibrationStep);
    }
    
}


void CalibrationXmlReader::readStep(StepType& step, CalibrationStep& calibrationStep) {
    
    DoFsType& myDofs(step.dofs());
    DoFsType::iterator dofIt = myDofs.begin();
    for(dofIt; dofIt != myDofs.end(); ++dofIt)
        calibrationStep.pushDofName(*dofIt);
    ObjectiveFunctionType& myObjFuc(step.objectiveFunction());
    ObjectiveFunctionElementType& myObjFuncMinTorqueError(myObjFuc.minimizeTorqueError());
  //  if (myObjFuncMinTorqueError.present())
    calibrationStep.setObjectiveFunction(CalibrationCfg::MinimizeTorqueError);
    
    //add computation torque mode
    ComputationModeType& myCompMode(step.computationMode());
    StepType::computationMode_type::default_optional& myCompModeDefaultOpt(myCompMode.default_());
    if(myCompModeDefaultOpt.present())
        calibrationStep.setComputationMode(CalibrationCfg::Default);
    else
        calibrationStep.setComputationMode(CalibrationCfg::Fast);

    StepType::strengthCoefficients_ShapeFactor_C1_C2_TendonSLackLength_single_optional& myHardCodedParamPolicyOpt(step.strengthCoefficients_ShapeFactor_C1_C2_TendonSLackLength_single());
    StepType::parameterSet_optional& myParamSetOpt(step.parameterSet());
    
    if(myHardCodedParamPolicyOpt.present())
        calibrationStep.setParametersPolicy(CalibrationCfg::StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single);
    else if(myParamSetOpt.present()) {       
        calibrationStep.setParametersPolicy(CalibrationCfg::ParameterSet);
        
        StepType::parameterSet_type::parameter_sequence& parameterSequence(step.parameterSet()->parameter());
        StepType::parameterSet_type::parameter_iterator parIt = parameterSequence.begin();
        for(parIt; parIt != parameterSequence.end(); ++parIt) {
            Parameter parameter;
            readParameter(*parIt, parameter);
            calibrationStep.pushParameter(parameter);
        }        
    }
        
}


void CalibrationXmlReader::readParameter(ParameterType& parameterType, Parameter& parameter) {

    parameter.setName(parameterType.name());
    ParameterType::muscleGroups_optional& myMuscleGroupsOpt(parameterType.muscleGroups());
    ParameterType::global_optional& myGlobaleParameterOpt(parameterType.global());
    ParameterType::single_optional& mySingleParameterOpt(parameterType.single());
    if(mySingleParameterOpt.present())
        parameter.setAssignment(Parameter::Single);
    else if (myGlobaleParameterOpt.present())
        parameter.setAssignment(Parameter::Global);
    else if (myMuscleGroupsOpt.present()) {
        parameter.setAssignment(Parameter::Grouped);
        MuscleGroupsType::muscles_sequence& musclesSequence(parameterType.muscleGroups()->muscles()); 
        MuscleGroupsType::muscles_iterator mgIt = musclesSequence.begin();

        for(mgIt; mgIt != musclesSequence.end(); ++mgIt) {
            Parameter::MuscleNames muscleNames;
            for(MuscleListType::iterator it = mgIt->begin(); it != mgIt->end(); ++it)
                muscleNames.push_back(*it);
            parameter.pushMuscleGroup(muscleNames);
        }
    }
    
    ParameterType::absolute_optional myAbsoluteRangeOpt(parameterType.absolute());
    ParameterType::relativeToSubjectValue_optional& myRelativeRangeOpt(parameterType.relativeToSubjectValue());
    TwoDoublesType::iterator it;
    if(myAbsoluteRangeOpt.present()) {
        parameter.setBoundariesType(Parameter::Absolute);
        ParameterType::absolute_type::range_type& absoluteRange(parameterType.absolute()->range());
        it = absoluteRange.begin(); 
    }
    else if(myRelativeRangeOpt.present()) {
        parameter.setBoundariesType(Parameter::RelativeToSubjectValue);
        ParameterType::relativeToSubjectValue_type::range_type& relativeRange(parameterType.relativeToSubjectValue()->range());
        it = relativeRange.begin(); 
    }
    
    parameter.setLowerAndUpperLimits(*it, *(it+1));   
}


void CalibrationXmlReader::readCalibrationTrialsDirectory() {

	trialsDirectory_ = calibrationPointer_->trialsDirectory();
}

void CalibrationXmlReader::readCalibrationTrialList() {
    
    CalibrationType::trialSet_type& myTrialSet(calibrationPointer_->trialSet());
    TrialSetType::iterator it = myTrialSet.begin();
    for(it; it != myTrialSet.end(); ++it)
        calibrationTrials_.push_back(*it);
}



NMSModelCfg::RunMode CalibrationXmlReader::getNMSmodelRunMode() const {

   return static_cast<NMSModelCfg::RunMode>(runMode_);
}


bool CalibrationXmlReader::popNextCalibrationStep(CalibrationStep& calibrationStep) {

    if(calibrationSteps_.empty())
        return 0;
    
    calibrationStep = calibrationSteps_.front();
    calibrationSteps_.pop_front();
    return 1;
}


CalibrationCfg::OptimizationAlgorithm CalibrationXmlReader::getOptimizationAlgorithm() const {

    return static_cast<CalibrationCfg::OptimizationAlgorithm>(optimizationAlgorithm_);
}


void CalibrationXmlReader::getOptimizationAlgorithmParameters(SimulatedAnnealingParameters& parameters) const {

    parameters = simanParameters_;
}


void CalibrationXmlReader::getTrialsDirectory(string& trialsDirectory) const {

	trialsDirectory = trialsDirectory_;
}

void CalibrationXmlReader::getCalibrationTrials(std::list<string>& trials) const {

    trials = calibrationTrials_;
}



void CalibrationStep::setObjectiveFunction(unsigned int objectiveFunction) {
    
    objectiveFunction_ = objectiveFunction;
}


void CalibrationStep::setParametersPolicy(unsigned int parametersPolicy) {
    
    parametersPolicy_ = parametersPolicy;
}


void CalibrationStep::setComputationMode(unsigned int computationMode) {
    
    computationMode_ = computationMode;
}


CalibrationCfg::Step CalibrationStep::getStepCfg() const {

    return static_cast<CalibrationCfg::Step>(objectiveFunction_+parametersPolicy_+computationMode_);
}


void CalibrationStep::getDofNames(std::list< string >& dofNames) const {

    dofNames = dofNames_;
}


void CalibrationStep::getDofNames(std::vector< string >& dofNames) const {

    dofNames.assign(dofNames_.begin(), dofNames_.end());
}



void CalibrationStep::getParameterSet(ParameterSet& parameterSet) const {

    parameterSet = parameterSet_;
}


void CalibrationStep::pushParameter(const Parameter& parameter) {

    parameterSet_.push_back(parameter);
}


void CalibrationStep::pushDofName(string& dofName) {

    dofNames_.push_back(dofName);
}


std::ostream& operator<< (std::ostream& output, const CalibrationXmlReader& rhs) {
 
    output << "Calibration configuration\n";
    output << " - Algorithm\n";
    switch(rhs.optimizationAlgorithm_) {
        case CalibrationCfg::Siman:
            output << " -- Simulated Annealing\n"; 
            output << " --- noEpsilon " << rhs.simanParameters_.noEpsilon << std::endl;
            output << " --- NS " << rhs.simanParameters_.NS << std::endl;
            output << " --- NT " << rhs.simanParameters_.NT << std::endl;
            output << " --- maxNoEval " << rhs.simanParameters_.maxNoEval << std::endl;
            output << " --- rt " << rhs.simanParameters_.rt << std::endl;
            output << " --- T " << rhs.simanParameters_.T << std::endl;
            output << " --- epsilon " << rhs.simanParameters_.epsilon << std::endl;
            break;
        default:
            output << " -- Not Found\n"; 
    }
    output << " - NMSmodel\n";
    output << " - CalibrationSteps\n";
    std::list<CalibrationStep>::const_iterator csIt = rhs.calibrationSteps_.begin();
    for(csIt; csIt != rhs.calibrationSteps_.end(); ++csIt)
        output << " -- Step\n" << *(csIt);
    output << " - Trials\n";
    for(std::list<std::string>::const_iterator tIt = rhs.calibrationTrials_.begin(); tIt != rhs.calibrationTrials_.end(); ++tIt)
        output << " -- " << *tIt << std::endl;
    output << std::endl;
    return output;
}


std::ostream& operator<< (std::ostream& output, const CalibrationStep& rhs) {

    output << " --- DoFs: ";
    std::list<std::string>::const_iterator dIt = rhs.dofNames_.begin();
    for(dIt; dIt != rhs.dofNames_.end(); ++dIt)
        output << *dIt << " ";
    output << std::endl;
    output << " --- Objective Function: ";
    switch(rhs.objectiveFunction_) {
        case (CalibrationCfg::MinimizeTorqueError):
            output << "Minimize Torque Error\n";
            break;
        default:
            output << "Not Found\n";
            break;
    }

    output << " --- Computation Mode: ";
    switch(rhs.computationMode_) {
        case (CalibrationCfg::Default):
            output << "Default\n";
            break;
        case (CalibrationCfg::Fast):
            output << "Fast\n";
            break;
        default:
            output << "Not Found\n";
            break;
    }

    output << " --- Parameters Policy: ";
    switch(rhs.parametersPolicy_) {
        case (CalibrationCfg::StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single):
            output << "Hardcoded policy\n";
            break;
        case (CalibrationCfg::ParameterSet):
            output << "ParameterSet\n";
            for(ParameterSet::const_iterator it = rhs.parameterSet_.begin(); it != rhs.parameterSet_.end(); ++it)
                output << *it;
            break;
        default:
            output << "Not Found\n";
            break;
    }    
    return output;
}




























