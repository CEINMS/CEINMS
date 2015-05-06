#include "QueuesToContactForces.h"
#include <string>

namespace OpenSim {

    template<typename Q>
    ExternalForceFromQueue<Q>::ExternalForceFromQueue() : Force()
    {
        setNull();
    }

    template<typename Q>
    ExternalForceFromQueue<Q>::ExternalForceFromQueue
        (Q& dataSource,
        const std::string& appliedToBodyName,
        const std::string& forceExpressedInBodyName,
        const std::string& pointExpressedInBodyName) :
        dataSource_(dataSource),
        appliedToBodyName_(appliedToBodyName),
        forceExpressedInBodyName_(forceExpressedInBodyName),
        pointExpressedInBodyName_(pointExpressedInBodyName), 
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr)
    {
        
    }


    template<typename Q>
    ExternalForceFromQueue<Q>::ExternalForceFromQueue(Q& dataSource, SimTK::Xml::Element& node) : 
        dataSource_(dataSource),
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr)
    {
        updateFromXMLNode(node);

    }


    template<typename Q>
    void ExternalForceFromQueue<Q>::updateFromXMLNode(SimTK::Xml::Element& aNode, int versionNumber)
    {


    }

    template<typename Q>
    void ExternalForceFromQueue<Q>::connectToModel(Model& model) {

        Super::connectToModel(model);

        // hook up body pointers from names
        appliedToBody_ = &_model->updBodySet().get(appliedToBodyName_);
        forceExpressedInBody_ = &_model->updBodySet().get(forceExpressedInBodyName_);
        pointExpressedInBody_ = &_model->updBodySet().get(pointExpressedInBodyName_);

    }


    template<typename Q>
    void ExternalForceFromQueue<Q>::computeForce(
        const SimTK::State& state,
        SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
        SimTK::Vector& generalizedForces) const
    {
        double time(state.getTime());
        Q::ForceData forceData(dataSource_.pop());
        while (forceData.time < time)
            forceData = dataSource_.pop();

        const SimbodyEngine& engine = getModel().getSimbodyEngine();

        engine.transform(state, *forceExpressedInBody_, forceData.data.grf, engine.getGroundBody(), forceData.data.grf);
        engine.transformPosition(state, *pointExpressedInBody_, forceData.data.cop, *appliedToBody_, forceData.data.cop);
        applyForceToPoint(state, *appliedToBody_, forceData.data.cop, forceData.data.grf, bodyForces);
        engine.transform(state, *forceExpressedInBody_, forceData.data.moments, engine.getGroundBody(), forceData.data.moments);
        applyTorque(state, *appliedToBody_, forceData.data.moments, bodyForces);
    }
    
}


namespace ceinms {



    template<typename Q1, typename Q2, typename Q3, typename Q4>
    template<typename NMSmodelT>
    QueuesToContactForces<Q1, Q2, Q3, Q4>::QueuesToContactForces(
        Q1& inputStateQueue,
        Q2& inputExternalForcesQueue,
        Q3& inputMuscleForcesQueue,
        Q4& outputContactForcesQueue,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,
        const std::vector< std::string >& jointsToLog) :
        inputStateQueue_(inputStateQueue),
        inputExternalForcesQueue_(inputExternalForcesQueue_),
        inputMuscleForcesQueue_(inputMuscleForcesQueue_),
        outputContactForcesQueue_(outputContactForcesQueue),
        model_(osimModelFilename),
        jointsToLog_(jointsToLog)

    {
        subject.getDoFNames(dofNames_);
        subject.getMusclesNames(musclesNames_);

        inputStateQueue_.subscribe();
        inputExternalForcesQueue_.subscribe();
        inputMuscleForcesQueue_.subscribe();
        
    }



    template<typename Q1, typename Q2, typename Q3, typename Q4>
    QueuesToContactForces<Q1, Q2, Q3, Q4>::operator()() {

        Q2::type 

    }
}