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

#ifndef ceinms_ExcitationGeneratorFromXml_h
#define ceinms_ExcitationGeneratorFromXml_h

#include <string>
#include <vector>

namespace ceinms {
    class ExcitationGeneratorFromXml{

        struct Component
        {
            unsigned index; //index of this component in the inputSignals list
            double weight;
        };

    public:
        ExcitationGeneratorFromXml(const std::string& xmlFilename = "excitationGenerator.xml");
        inline bool checkInputSignalsNames(const std::vector<std::string>& inputSignalsNames) const {
            return ((inputSignalsNames == inputSignalsNames_));
        }
        void convert(const std::vector<double>& inputSignals, std::vector<double>& excitations) const;
        int getNoInputSignals() const { return noInputSignals_; }
        int getNoExcitations() const { return noExcitations_; }
        void getInputSignalsNames(std::vector<std::string>& inputSignalsNames) const { inputSignalsNames = inputSignalsNames_; }
        void getExcitationsNames(std::vector<std::string>& excitationsNames) const { excitationsNames = excitationsNames_; }
        const std::vector<std::string>& getInputSignalsNames() const { return inputSignalsNames_; }
        const std::vector<std::string>& getExcitationsNames() const { return excitationsNames_; }

    private:
        unsigned noInputSignals_;
        unsigned noExcitations_;
        std::vector< std::vector<Component> > weights_;
        std::vector<std::string> inputSignalsNames_;
        std::vector<std::string> excitationsNames_;

    };
}
#endif
