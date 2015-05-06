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

#ifndef ceinms_QueuesToContactForces_h
#define ceinms_QueuesToContactForces_h

#include "Queue.h"

#include <OpenSim/OpenSim.h>

namespace OpenSim {


    //check for latch
    template<typename Q>
    class  ExternalForceFromQueue : public Force{

    public:
        ExternalForceFromQueue();
        ExternalForceFromQueue(Q& dataSource,
            const std::string& appliedToBodyName,
            const std::string& forceExpressedInBodyName,
            const std::string& pointExpressedInBodyName);
        explicit ExternalForceFromQueue(Q& dataSource, SimTK::Xml::Element& aNode);
        virtual void updateFromXMLNode(SimTK::Xml::Element& aNode, int versionNumber = -1) override;

    protected:
        void connectToModel(Model& model) override;
        virtual void computeForce(const SimTK::State& state,
            SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
            SimTK::Vector& generalizedForces) const override;

    private:
        void setNull();
        void constructProperties();

        Q& dataSource_;

        bool appliesForce_;
        bool specifiesPoint_;
        bool appliesTorque_;
        std::string appliedToBodyName_;
        std::string forceExpressedInBodyName_;
        std::string pointExpressedInBodyName_;
    };

}

namespace ceinms {
    template<typename Q1, typename Q2, typename Q3, typename Q4>
    class QueuesToContactForces {
    public:
        QueuesToContactForces() = delete;
        QueuesToContactForces(const QueuesToContactForces&) = delete;
        QueuesToContactForces& operator=(const QueuesToContactForces&) = delete;

        template <typename NMSmodelT>
        QueuesToContactForces(
            Q1& inputStateQueue,
            Q2& inputExternalForcesQueue,
            Q3& inputMuscleForcesQueue,
            Q4& outputContactForcesQueue,
            const NMSmodelT& subject,
            const std::string& osimModelFilename,
            const std::string& externalLoadsXmlFilename,    //this is the same as openSim
            const std::vector< std::string >& jointsToLog);

        ~QueuesToContactForces() = default;
        void operator()();

    private:
        Q1& inputStateQueue_;
        Q2& inputExternalForcesQueue_;
        Q3& inputMuscleForcesQueue_;
        Q4& outputContactForcesQueue_;
        std::vector<std::string> jointsToLog_, dofNames_, muscleNames;
        OpenSim::Model model_;

        void parseExternalLoadsXml(const std::string& externalLoadsXmlFilename);
    };
}

//#include "QueuesToContactForces.cpp"
#endif