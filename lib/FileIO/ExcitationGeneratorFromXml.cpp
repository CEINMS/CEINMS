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
 * Author(s): C. Pizzolato, E. Ceseracciu                                     *
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

#include "ceinms/ExcitationGeneratorFromXml.h"
#include "excitationGenerator.hxx"
using ExcitationGeneratorXsd::excitationGenerator;
using ExcitationGeneratorXsd::ExcitationListType;
using ExcitationGeneratorXsd::ExcitationType;

#include "ceinms/ExcitationGeneratorXmlReader.h"
#include <vector>
using std::vector;
#include <string>
using std::string;

#include <algorithm>
using std::find;
#include <iostream>
#include <stdlib.h>

namespace ceinms {
    ExcitationGeneratorFromXml::ExcitationGeneratorFromXml(const std::string& xmlFilename) {

        try {

            ExcitationGeneratorXmlReader sxr(xmlFilename);
            std::auto_ptr<excitationGenerator> generatorPtr = sxr.getExcitationGenerator();

            excitationGenerator::inputSignals_type& inputSignals(generatorPtr->inputSignals());
            for (excitationGenerator::inputSignals_type::iterator it(inputSignals.begin()); it != inputSignals.end(); ++it)
                inputSignalsNames_.push_back(*it);

            ExcitationListType::excitation_sequence& excitations(generatorPtr->mapping().excitation());
            for (auto it : excitations)
            {
                excitationsNames_.push_back(it.id());
                ExcitationType::input_sequence inputSequence(it.input());
                std::vector<Component> excitationComponentsVector/*(inputSequence.size())*/;
                for (auto inputIt : inputSequence)
                {
                    Component currentComponent;
                    currentComponent.weight = inputIt.weight();
                    std::vector<std::string>::iterator positionInInputSignalsList = std::find(inputSignalsNames_.begin(), inputSignalsNames_.end(), inputIt.data());
                    if (positionInInputSignalsList == inputSignalsNames_.end())
                    {
                        std::cout << "Excitation generator: could not find " << inputIt.data() << " input signal" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    currentComponent.index = positionInInputSignalsList - inputSignalsNames_.begin();
                    excitationComponentsVector.push_back(currentComponent);
                }
                weights_.push_back(excitationComponentsVector);
            }

            noInputSignals_ = inputSignalsNames_.size();
            noExcitations_ = excitationsNames_.size();

        }
        catch (const xml_schema::exception& e) {
            std::cout << e << std::endl;
            exit(EXIT_FAILURE);
        }

    }


    void ExcitationGeneratorFromXml::convert(const std::vector< double >& inputSignals, std::vector< double >& excitations) const {

        if (inputSignals.size() != noInputSignals_) {
            std::cout << "I need " << noInputSignals_ << " input signals for excitation generation, but I got " << inputSignals.size() << " muscles!\n";
            exit(EXIT_FAILURE);
        }

        if (excitations.size() != noExcitations_) {
            std::cout << "I should provide " << noExcitations_ << " excitations, but you asked for " << excitations.size() << std::endl;
            exit(EXIT_FAILURE);
        }

        excitations.clear();
        excitations.resize(noExcitations_);
        for (unsigned i = 0; i < noExcitations_; ++i) {

            double result = 0;
            for (auto inputIt : weights_.at(i))
                result += inputIt.weight * inputSignals.at(inputIt.index);

            excitations.at(i) = result;
        }
    }

}
