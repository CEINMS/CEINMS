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

#ifndef ceinms_CalibrationXmlReader_h
#define ceinms_CalibrationXmlReader_h

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

namespace ceinms {

    typedef std::list<Parameter> ParameterSet;

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
}
#endif
