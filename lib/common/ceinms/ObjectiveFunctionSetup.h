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

#ifndef ceinms_ObjectiveFunctionSetup_h
#define ceinms_ObjectiveFunctionSetup_h

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>

namespace ceinms {
 
    enum class CalibrationObjectiveFunction {
        TorqueError,
        TorqueErrorNormalised,
        MaxPeak,
        MaxSumPeak,
        MaxPeakGenericContactForces,
        Penalty,
        None
    };

    enum class CalibrationTargetType {
        MuscleForce,
        Torque,
        Activation,
        NormalisedFibreLength,
        NormalisedFibreVelocity,
        PennationAngle,
        ScalarContactForce,
        GenericContactForce,
        None
    };
    /*
    Generally, objective function should work like
    f = weight * sum(target^exponent)
    */
    class ObjectiveFunctionSetup {
    public:
        ObjectiveFunctionSetup() : 
            weight_(1.), 
            exponent_(1.), 
            objectiveFunction_(CalibrationObjectiveFunction::None),
            targetType_(CalibrationTargetType::None) {}

        ObjectiveFunctionSetup(
            CalibrationObjectiveFunction objectiveFunction, 
            CalibrationTargetType targetType,
            std::vector<std::string> targetNames,
            double weight, double exponent):
            weight_(weight),
            exponent_(exponent),
            objectiveFunction_(objectiveFunction),
            targetType_(targetType),
            targetNames_(targetNames) {}

        ObjectiveFunctionSetup(
            CalibrationObjectiveFunction objectiveFunction,
            CalibrationTargetType targetType,
            std::vector<std::string> targetNames,
            std::vector<std::string> targetNamesRef,
            double weight, double exponent) :
            weight_(weight),
            exponent_(exponent),
            objectiveFunction_(objectiveFunction),
            targetType_(targetType),
            targetNames_(targetNames),
            targetNamesRef_(targetNamesRef) {}

        virtual ~ObjectiveFunctionSetup() = default;

        virtual void setWeight(double weight) {
            weight_ = weight;
        }
        virtual void setExponent(double exponent) {
            exponent_ = exponent;
        }
        virtual void setCalibrationObjectiveFunction(CalibrationObjectiveFunction objectiveFunction) {
            objectiveFunction_ = objectiveFunction;
        }

        virtual void setCalibrationTargetType(CalibrationTargetType calibrationTargetType) {
            targetType_ = calibrationTargetType;
        }

        virtual void setTargetNames(std::vector<std::string> targetNames) {
            targetNames_ = targetNames;
        }

        virtual void setTargetNames(std::string targetNames) {
            targetNames_ = { targetNames };
        }

        virtual void setTargetNamesRef(std::vector<std::string> targetNames) {
            targetNamesRef_ = targetNames;
        }

        virtual void setTargetNamesRef(std::string targetNames) {
            targetNamesRef_ = { targetNames };
        }

        virtual double getWeight() const {
            return weight_;
        }

        virtual double getExponent() const {
            return exponent_;
        }

        virtual CalibrationObjectiveFunction getCalibrationObjectiveFunction() const {
            return objectiveFunction_;
        }

        virtual CalibrationTargetType getCalibrationTargetType() const {
            return targetType_;
        }

        virtual  std::vector<std::string> getTargetNames() const {
            return targetNames_;
        }

        virtual  std::vector<std::string> getTargetNamesRef() const {
            return targetNamesRef_;
        }

        friend std::ostream& operator<< (std::ostream& os, const ObjectiveFunctionSetup& obj) {
            obj.print(os);
            return os;
        }

    protected:
        virtual void print(std::ostream& os) const {
            using ObjFunMap = std::map<CalibrationObjectiveFunction, std::string>;

            ObjFunMap objFunMap = {
                { CalibrationObjectiveFunction::MaxSumPeak,"MaxSumPeak" },
                { CalibrationObjectiveFunction::MaxPeak,"MaxPeak" },
                { CalibrationObjectiveFunction::MaxPeakGenericContactForces,"MaxPeakGenericContactForces" },
                { CalibrationObjectiveFunction::Penalty,"Penalty" },
                { CalibrationObjectiveFunction::TorqueErrorNormalised,"TorqueErrorNormalised" },
                { CalibrationObjectiveFunction::None,"Not Found" }
            };

            using TargetTypeMap = std::map<CalibrationTargetType, std::string>;

            TargetTypeMap targetType = {
                { CalibrationTargetType::MuscleForce,"MuscleForce" },
                { CalibrationTargetType::Activation,"Activation" },
                { CalibrationTargetType::GenericContactForce,"Generic Contact Force" },
                { CalibrationTargetType::NormalisedFibreLength,"Normalised Fibre Length" },
                { CalibrationTargetType::NormalisedFibreVelocity,"Normalised Fibre Velocity" },
                { CalibrationTargetType::PennationAngle,"Pennation Angle" },
                { CalibrationTargetType::ScalarContactForce,"Scalar Contact Force" },
                { CalibrationTargetType::Torque,"Torque" },
                { CalibrationTargetType::None,"Not Found" }
            };

            os << " Objective Function: " << objFunMap[objectiveFunction_] << "\n";
            os << " - Targets Type: " << targetType[targetType_] << "\n";
            os << " - Targets: ";
            for (auto& e : getTargetNames())
                os << e << " ";
            os << "\n";
            os << " - Weight: " << weight_ << "\n";
            os << " - Exponent: " << exponent_ << "\n";

        }

    private:
        CalibrationObjectiveFunction objectiveFunction_;
        CalibrationTargetType targetType_;
        std::vector<std::string> targetNames_;
        std::vector<std::string> targetNamesRef_;
        double weight_, exponent_;
    };

	//This is used to substitute the keyword `all` with the actual names of the targets
		//return true if successfully updated the targets names
	template<typename NMSmodelT>
	bool updateTargetsFromModel(ObjectiveFunctionSetup& objFunctionSetup, NMSmodelT& model) {
		if (objFunctionSetup.getTargetNames().empty())
			return false;
		if (objFunctionSetup.getTargetNames().at(0) != "all")
			return false;

		switch (objFunctionSetup.getCalibrationTargetType()) {
        case CalibrationTargetType::MuscleForce:
		case CalibrationTargetType::Activation:
		case CalibrationTargetType::NormalisedFibreLength:
		case CalibrationTargetType::NormalisedFibreVelocity:
			objFunctionSetup.setTargetNames(model.getMuscleNames());
            if (objFunctionSetup.getTargetNamesRef().size() > 0) {
                if (objFunctionSetup.getTargetNamesRef()[0] == "all")
                        objFunctionSetup.setTargetNamesRef(model.getDoFNames());
            }
			break;
		case CalibrationTargetType::Torque:
			objFunctionSetup.setTargetNames(model.getDoFNames());
			break;
		case CalibrationTargetType::ScalarContactForce:
		case CalibrationTargetType::GenericContactForce:
			std::cout << "Keyword `all` is not allowed "
				"to specify which contact forces to target during the calibration." << std::endl;
			exit(EXIT_FAILURE);
		}
		return true;
	}

    using ObjectiveFunctionSetupList = std::vector<std::shared_ptr<ObjectiveFunctionSetup>>;

	template<typename NMSmodelT>
	bool updateTargetsFromModel(ObjectiveFunctionSetupList& objFunctionSetupList, NMSmodelT& model) {
		bool success = true;
		for (auto& e : objFunctionSetupList)
			success &= updateTargetsFromModel(*e, model);
		return success;
	}

    class PenaltyObjectiveFunctionSetup : public ObjectiveFunctionSetup {
    public:
        PenaltyObjectiveFunctionSetup() :
            ObjectiveFunctionSetup(),
            xL_(0), xR_(1000) {}

        PenaltyObjectiveFunctionSetup(
            CalibrationTargetType targetType,
            std::vector<std::string> targetNames,
            double weight, double exponent,
            double minVal, double maxVal) :
            ObjectiveFunctionSetup(
                CalibrationObjectiveFunction::Penalty,
                targetType, targetNames, weight, exponent),
            xL_(minVal), xR_(maxVal) {}

        void setInterval(double minVal, double maxVal) {
            xL_ = minVal;
            xR_ = maxVal;
        }

        double getMinValue() const {
            return xL_;
        }

        double getMaxValue() const {
            return xR_;
        }

    protected:
        virtual void print(std::ostream& os) const {
            ObjectiveFunctionSetup::print(os);
            os << " - Range: " << xL_ << ", " << xR_ << "\n";
        }

    private:
        double xL_, xR_;
    };
}

#endif
