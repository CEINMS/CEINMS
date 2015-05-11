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

#include "ceinms/concurrency/Queue.h"
#include "ceinms/QueueData.h"
#include <memory>
#include <OpenSim/OpenSim.h>
#include <ostream>

namespace ceinms {

    struct ExternalForceData {
        unsigned id;
        SimTK::Vec3 cop;
        SimTK::Vec3 grf;
        SimTK::Vec3 moments;
    };


    std::ostream& operator<< (std::ostream& os, const ExternalForceData& forceData) {

        os << "cop: " << forceData.cop << std::endl;
        os << "grf: " << forceData.grf << std::endl;
        os << "moments: " << forceData.moments << std::endl;
        return os;
    }
}

namespace OpenSim {

    class ExternalForceFromQueue : public OpenSim::ExternalForce{
    public:
        using ExternalForceQueue = ceinms::Concurrency::Queue < ceinms::QueueData<ceinms::ExternalForceData> > ;
        ExternalForceFromQueue() = delete;
        ExternalForceFromQueue(std::shared_ptr<ExternalForceQueue> externalForceQueue,
            const std::string& appliedToBodyName,
            const std::string& forceExpressedInBodyName,
            const std::string& pointExpressedInBodyName);
        ExternalForceFromQueue(std::shared_ptr<ExternalForceQueue> externalForceQueue, SimTK::Xml::Element& aNode);
        virtual ~ExternalForceFromQueue();
    protected:
        virtual void computeForce(const SimTK::State& state,
            SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
            SimTK::Vector& generalizedForces) const override;
        void connectToModel(Model& model) override;
    private:
        bool isSubscribed;
        ceinms::ExternalForceData getForceDataFromPastValues(double time) const;
        ceinms::ExternalForceData getForceDataFromQueue(double time) const;
        std::shared_ptr<ExternalForceQueue> externalForceQueue_;
        mutable std::list<ceinms::QueueData<ceinms::ExternalForceData> > pastForces_;
        mutable double lastTime_;
        SimTK::ReferencePtr<const Body> appliedToBody_;
        SimTK::ReferencePtr<const Body> forceExpressedInBody_;
        SimTK::ReferencePtr<const Body> pointExpressedInBody_;
        mutable bool runCondition_;


    };

}

namespace ceinms {
   
    class GeneralisedCoordinates {
    private:
        size_t nCoordinates_;
        std::vector<double> q_, qd_, qdd_;

    public:
        GeneralisedCoordinates(size_t nCoordinates) :
            nCoordinates_(nCoordinates), 
            q_(nCoordinates, .0),
            qd_(nCoordinates, .0),
            qdd_(nCoordinates, .0)
            {}
        void setQ(const std::vector<double>& q) {
            q_ = q;
        }
        void setQd(const std::vector<double>& qd) {
            qd_ = qd;
        }
        void setQdd(const std::vector<double>& qdd) {
            qdd_ = qdd;
        }

        std::vector<double> getQ() const { return q_; }
        std::vector<double> getQd() const { return qd_; }
        std::vector<double> getQdd() const { return qdd_; }
        
    };


    template<typename Q1, typename Q2, typename Q3, typename Q4, typename L>
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
            std::vector<std::unique_ptr<L>>& latches,
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
        std::string modelFilename_;
        std::vector<std::unique_ptr<L>>& latches_;
        std::vector<std::string> jointsToLog_, dofNames_, musclesNames_;
        //OpenSim::Model model_;
        using QueueForce = Concurrency::Queue < QueueData<ExternalForceData> > ;
        std::vector<std::shared_ptr<QueueForce>> forceQueues_;
        std::vector<OpenSim::Force*> extForces_;
        void parseExternalLoadsXml(const std::string& externalLoadsXmlFilename);
    };
}

#include "QueuesToContactForces.cpp"
#endif