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



#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <ostream>
#include <vector>
using std::vector;

#include "ceinms/CalibrationXmlReader.h"
#include "ceinms/FileUtils.h"
#include "calibration.hxx"
#include "validation.h"
#include "calibration-schema.hxx"
using namespace CalibrationXsd;

namespace ceinms {

    CalibrationXmlReader::CalibrationXmlReader(const string& filename)
        :runMode_(0), optimizationAlgorithm_(0), tolerance_(1e-6) {

        try {
            std::unique_ptr<CalibrationType> calibrationPointer(parseAndValidate<CalibrationType>(filename, calibration_schema, sizeof(calibration_schema)));
            calibrationPointer_ = std::move(calibrationPointer);
        }
        catch (const xml_schema::exception& e) {
            cout << e << endl;
            exit(EXIT_FAILURE);
        }

        // save the location of input filename
        string::size_type dirSep = filename.rfind('/'); // Unix/Mac dir separator

        if (dirSep == string::npos)
            dirSep = filename.rfind('\\'); // DOS dir separator

        if (dirSep != string::npos) // if '_fileName' contains path information...
            filepath_ = filename.substr(0, dirSep + 1); // include trailing slashes

        readXml();
    }

    void CalibrationXmlReader::readXml() {
        readNMSmodelCfg();
        readOptimizationAlgorithmCfg();
        readCalibrationStepsCfg();
        readCalibrationTrialList();
    }

    void CalibrationXmlReader::readNMSmodelCfg() {
        cout << "readNMSmodelCfg\n";
        try {
            CalibrationType::NMSmodel_type& myModel(calibrationPointer_->NMSmodel());
            CalibrationType::NMSmodel_type::activation_type& myActivation(myModel.activation());
            CalibrationType::NMSmodel_type::activation_type::exponential_optional& myExpOpt(myActivation.exponential());
            CalibrationType::NMSmodel_type::activation_type::piecewise_optional& myPWOpt(myActivation.piecewise());

            if (myExpOpt.present())
                runMode_ += NMSModelCfg::ExponentialActivation;
            else if (myPWOpt.present())
                runMode_ += NMSModelCfg::PiecewiseActivation;
            else {
                cout << "invalid XML: ActivationType not found\n";
                exit(EXIT_FAILURE);
            }

            CalibrationType::NMSmodel_type::tendon_type& myTendon(myModel.tendon());
            CalibrationType::NMSmodel_type::tendon_type::stiff_optional& myStiffOpt(myTendon.stiff());
            CalibrationType::NMSmodel_type::tendon_type::integrationElastic_optional& myElsaticOpt(myTendon.integrationElastic());
            CalibrationType::NMSmodel_type::tendon_type::equilibriumElastic_optional& myElsaticBiSecOpt(myTendon.equilibriumElastic());

            if (myStiffOpt.present())
                runMode_ += NMSModelCfg::StiffTendon;
            else if (myElsaticOpt.present())
                runMode_ += NMSModelCfg::ElasticTendon;
            else if (myElsaticBiSecOpt.present())
            {
                runMode_ += NMSModelCfg::ElasticTendonBiSec;
                if (myElsaticBiSecOpt->tolerance().present())
                    tolerance_ = myElsaticBiSecOpt->tolerance().get();
            }
            else {
                cout << "invalid XML: TendonType not found\n";
                exit(EXIT_FAILURE);
            }

            CalibrationType::NMSmodel_type::type_type& myType(myModel.type());
            runMode_ += NMSModelCfg::OpenLoop;

            runMode_ += NMSModelCfg::Offline;

        }
        catch (const xml_schema::exception& e) {
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
        simanParameters_.rt = mySiman.rt();
        simanParameters_.T = mySiman.T();
        simanParameters_.NS = mySiman.NS();
        simanParameters_.NT = mySiman.NT();
        simanParameters_.epsilon = mySiman.epsilon();
        simanParameters_.maxNoEval = mySiman.maxNoEval();
    }

    void CalibrationXmlReader::readCalibrationStepsCfg() {
        CalibrationType::calibrationSteps_type& myCalibrationSteps(calibrationPointer_->calibrationSteps());
        CalibrationType::calibrationSteps_type::step_sequence& stepSequence(myCalibrationSteps.step());
        CalibrationType::calibrationSteps_type::step_iterator stepIt = stepSequence.begin();
        for (stepIt; stepIt != stepSequence.end(); ++stepIt) {
            CalibrationStep calibrationStep = readStep(*stepIt);
            calibrationSteps_.push_back(calibrationStep);
        }
    }

    CalibrationStep CalibrationXmlReader::readStep(const StepType& step) {
        using TargetTypeMap = std::map<string, CalibrationTargetType>;

        TargetTypeMap targetType = {
            { "muscleForce", CalibrationTargetType::MuscleForce},
            { "activation", CalibrationTargetType::Activation},
            { "genericContactForce", CalibrationTargetType::GenericContactForce },
            { "normalisedFibreLength", CalibrationTargetType::NormalisedFibreLength},
            { "normalisedFibreVelocity", CalibrationTargetType::NormalisedFibreVelocity},
            { "pennationAngle", CalibrationTargetType::PennationAngle },
            { "scalarContactForce", CalibrationTargetType::ScalarContactForce},
            { "torque", CalibrationTargetType::Torque },
            { "none", CalibrationTargetType::None }
        };

        CalibrationStep calibrationStep;
        const ObjectiveFunctionType& myObjFuc(step.objectiveFunction());

        if (myObjFuc.minimizeTorqueError().present()){
            const TokenListType& myDofs(step.dofs().get());
            TokenListType::const_iterator dofIt = myDofs.begin();
            vector<string> dofNames;
            for (dofIt; dofIt != myDofs.end(); ++dofIt)
                dofNames.push_back(*dofIt);
            std::shared_ptr<ObjectiveFunctionSetup> obf(
                new ObjectiveFunctionSetup(
                    CalibrationObjectiveFunction::TorqueErrorNormalised,
                    CalibrationTargetType::Torque,
                    dofNames, 1, 1));
            calibrationStep.objFunctionsSetup_.push_back(obf);
        }

        for (auto& objElement : myObjFuc.torqueErrorNormalised()) {
            vector<string> targets;
            for (auto& t : objElement.targets())
                targets.push_back(t);
            std::shared_ptr<ObjectiveFunctionSetup> obf(
                new ObjectiveFunctionSetup(
                    CalibrationObjectiveFunction::TorqueErrorNormalised,
                    CalibrationTargetType::Torque,
                    targets, objElement.weight().get(), objElement.exponent().get()));
            calibrationStep.objFunctionsSetup_.push_back(obf);
        }

        for (auto& objElement : myObjFuc.maxPeak()) {
            vector<string> targets;
            for (auto& t : objElement.targets())
                targets.push_back(t);

            std::shared_ptr<ObjectiveFunctionSetup> obf(
                new ObjectiveFunctionSetup(
                    CalibrationObjectiveFunction::MaxPeak,
                    targetType[objElement.targetsType()],
                    targets, objElement.weight().get(), objElement.exponent().get()));
            calibrationStep.objFunctionsSetup_.push_back(obf);
        }

        for (auto& objElement : myObjFuc.penalty()) {
            vector<string> targets;
            for (auto& t : objElement.targets())
                targets.push_back(t);

            std::shared_ptr<ObjectiveFunctionSetup> obf(
                new PenaltyObjectiveFunctionSetup(
                    targetType[objElement.targetsType()],
                    targets, objElement.weight().get(), objElement.exponent().get(),
                    *objElement.range().begin(), *(objElement.range().begin()+1)));
            calibrationStep.objFunctionsSetup_.push_back(obf);
        }
        
        const StepType::parameterSet_type::parameter_sequence& parameterSequence = step.parameterSet().parameter();
        for (auto& par : parameterSequence) {
            Parameter parameter =  readParameter(par);
            calibrationStep.parameterSet_.push_back(parameter);
        }
        return calibrationStep;
    }

    Parameter CalibrationXmlReader::readParameter(const ParameterType& parameterType) {
        Parameter parameter;
        parameter.setName(parameterType.name());
        const ParameterType::muscleGroups_optional& myMuscleGroupsOpt(parameterType.muscleGroups());
        const ParameterType::global_optional& myGlobaleParameterOpt(parameterType.global());
        const ParameterType::single_optional& mySingleParameterOpt(parameterType.single());
        if (mySingleParameterOpt.present()) {
            parameter.setAssignment(Parameter::Single);
            auto it(parameterType.single()->begin());
            for (it; it != parameterType.single()->end(); ++it)
                parameter.pushMuscleGroup(Parameter::MuscleNames{ *it });

        }
        else if (myGlobaleParameterOpt.present())
            parameter.setAssignment(Parameter::Global);
        else if (myMuscleGroupsOpt.present()) {
            parameter.setAssignment(Parameter::Grouped);
            const MuscleGroupsType::muscles_sequence& musclesSequence(parameterType.muscleGroups()->muscles());
            for (auto& ms : musclesSequence) {
                std::list<string> muscleNames;
                for (auto& name : ms)
                    muscleNames.push_back(name);
                parameter.pushMuscleGroup(muscleNames);
            }
        }

        ParameterType::absolute_optional myAbsoluteRangeOpt(parameterType.absolute());
        const ParameterType::relativeToSubjectValue_optional& myRelativeRangeOpt(parameterType.relativeToSubjectValue());
        TwoDoublesType::const_iterator it;
        if (myAbsoluteRangeOpt.present()) {
            parameter.setBoundariesType(Parameter::Absolute);
            const ParameterType::absolute_type::range_type& absoluteRange(parameterType.absolute()->range());
            it = absoluteRange.begin();
        }
        else if (myRelativeRangeOpt.present()) {
            parameter.setBoundariesType(Parameter::RelativeToSubjectValue);
            const ParameterType::relativeToSubjectValue_type::range_type& relativeRange(parameterType.relativeToSubjectValue()->range());
            it = relativeRange.begin();
        }

        parameter.setLowerAndUpperLimits(*it, *(it + 1));
        return parameter;
    }

    void CalibrationXmlReader::readCalibrationTrialList() {
        CalibrationType::trialSet_type& myTrialSet(calibrationPointer_->trialSet());
        TrialSetType::iterator it = myTrialSet.begin();
        for (it; it != myTrialSet.end(); ++it)
        {
            string fileName = (*it);
            FileUtils::trim(fileName);
            if (FileUtils::isAbsolute(fileName.c_str()))
                calibrationTrials_.push_back(fileName);
            else
                calibrationTrials_.push_back(filepath_ + fileName);
        }
    }

    NMSModelCfg::RunMode CalibrationXmlReader::getNMSmodelRunMode() const {
        return static_cast<NMSModelCfg::RunMode>(runMode_);
    }

    bool CalibrationXmlReader::popNextCalibrationStep(CalibrationStep& calibrationStep) {
        if (calibrationSteps_.empty())
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

    void CalibrationXmlReader::getCalibrationTrials(std::list<string>& trials) const {
        trials = calibrationTrials_;
    }

    std::ostream& operator<< (std::ostream& output, const CalibrationXmlReader& rhs) {
        output << "Calibration configuration\n";
        output << " - Algorithm\n";
        switch (rhs.optimizationAlgorithm_) {
        case CalibrationCfg::Siman:
            output << " -- Simulated Annealing\n";
            output << " --- noEpsilon " << rhs.simanParameters_.noEpsilon << endl;
            output << " --- NS " << rhs.simanParameters_.NS << endl;
            output << " --- NT " << rhs.simanParameters_.NT << endl;
            output << " --- maxNoEval " << rhs.simanParameters_.maxNoEval << endl;
            output << " --- rt " << rhs.simanParameters_.rt << endl;
            output << " --- T " << rhs.simanParameters_.T << endl;
            output << " --- epsilon " << rhs.simanParameters_.epsilon << endl;
            break;
        default:
            output << " -- Not Found\n";
        }
        output << " - NMSmodel\n";
        output << " - CalibrationSteps\n";
        std::list<CalibrationStep>::const_iterator csIt = rhs.calibrationSteps_.begin();
        for (csIt; csIt != rhs.calibrationSteps_.end(); ++csIt) {
            output << " -- Step\n"; 
            for (auto& objfElement : csIt->objFunctionsSetup_)
                output << *objfElement;
        }
        output << " - Trials\n";
        for (std::list<string>::const_iterator tIt = rhs.calibrationTrials_.begin(); tIt != rhs.calibrationTrials_.end(); ++tIt)
            output << " -- " << *tIt << endl;
        output << endl;
        return output;
    }
}
