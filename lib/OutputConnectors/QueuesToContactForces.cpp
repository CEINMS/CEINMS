#include "QueuesToContactForces.h"
#include "JointContactFromOpensim.h"
#include <string>
#include <thread>
#include <limits>
#include <future>

#include <iostream> //

namespace OpenSim {

    ExternalForceFromQueue::ExternalForceFromQueue
        (ExternalForceQueue& externalForceQueue,
        const std::string& appliedToBodyName,
        const std::string& forceExpressedInBodyName,
        const std::string& pointExpressedInBodyName) :
        externalForceQueue_(externalForceQueue),
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr)
    {
        setAppliedToBodyName(appliedToBodyName);
        setForceExpressedInBodyName(forceExpressedInBodyName);
        setPointExpressedInBodyName(pointExpressedInBodyName);
        externalForceQueue_.subscribe();
    }


    ExternalForceFromQueue::ExternalForceFromQueue(ExternalForceQueue& externalForceQueue, SimTK::Xml::Element& node) :
        externalForceQueue_(externalForceQueue),
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr)
    {
        ExternalForce::updateFromXMLNode(node);
        externalForceQueue_.subscribe();
    }

    /*
    template<typename Q>
    void ExternalForceFromQueue<Q>::updateFromXMLNode(SimTK::Xml::Element& aNode, int versionNumber) {
        
        appliedToBodyName_ = aNode.element_begin("applied_to_body")->getValue();
        forceExpressedInBodyName_ = aNode.element_begin("force_expressed_in_body")->getValue();
        pointExpressedInBodyName_ = aNode.element_begin("point_expressed_in_body")->getValue();
            
    }
    */

    void ExternalForceFromQueue::computeForce(
        const SimTK::State& state,
        SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
        SimTK::Vector& generalizedForces) const
    {
        double time(state.getTime());
        ExternalForceQueue::type forceData(externalForceQueue_.pop());
        while (forceData.time < time)
            forceData = externalForceQueue_.pop();

        const SimbodyEngine& engine = getModel().getSimbodyEngine();
        engine.transform(state, *forceExpressedInBody_, forceData.data.grf, engine.getGroundBody(), forceData.data.grf);
        engine.transformPosition(state, *pointExpressedInBody_, forceData.data.cop, *appliedToBody_, forceData.data.cop);
        ExternalForce::applyForceToPoint(state, *appliedToBody_, forceData.data.cop, forceData.data.grf, bodyForces);
        engine.transform(state, *forceExpressedInBody_, forceData.data.moments, engine.getGroundBody(), forceData.data.moments);
        ExternalForce::applyTorque(state, *appliedToBody_, forceData.data.moments, bodyForces);
    }
 

    void ExternalForceFromQueue::connectToModel(Model& model) {

        Super::connectToModel(model);
        const std::string& appliedToBodyName = get_applied_to_body();
        const std::string& forceExpressedInBodyName = get_force_expressed_in_body();

        appliedToBody_ = &_model->updBodySet().get(appliedToBodyName);
        forceExpressedInBody_ = &_model->updBodySet().get(forceExpressedInBodyName);
        pointExpressedInBody_ = &_model->updBodySet().get(get_point_expressed_in_body());
    }

    ExternalForceFromQueue::~ExternalForceFromQueue() {
        
   //     externalForceQueue_.unsubscribe();
    }
}


namespace ceinms {

    template<typename Q1, typename Q2, typename Q3, typename Q4, typename L>
    template<typename NMSmodelT>
    QueuesToContactForces<Q1, Q2, Q3, Q4, L>::QueuesToContactForces(
        Q1& inputStateQueue,
        Q2& inputExternalForcesQueue,
        Q3& inputMuscleForcesQueue,
        Q4& outputContactForcesQueue,
        std::vector<std::unique_ptr<L>>& latches,
        const NMSmodelT& subject,
        const std::string& osimModelFilename,
        const std::string& externalLoadsXmlFilename,
        const std::vector< std::string >& jointsToLog) :
        inputStateQueue_(inputStateQueue),
        inputExternalForcesQueue_(inputExternalForcesQueue),
        inputMuscleForcesQueue_(inputMuscleForcesQueue),
        outputContactForcesQueue_(outputContactForcesQueue),
        latches_(latches),
        model_(osimModelFilename),
        jointsToLog_(jointsToLog)

    {
        subject.getDoFNames(dofNames_);
        subject.getMuscleNames(musclesNames_);

        parseExternalLoadsXml(externalLoadsXmlFilename);
    }


    template<typename Q1, typename Q2, typename Q3, typename Q4, typename L>
    void QueuesToContactForces<Q1, Q2, Q3, Q4, L>::parseExternalLoadsXml(const std::string& xmlFilename) {
        OpenSim::XMLDocument document(xmlFilename);
        SimTK::Xml::Element root = document.getRootDataElement();
        SimTK::Xml::element_iterator extForceElementIter = root.element_begin("objects")->element_begin("ExternalForce");
        
        for (extForceElementIter; extForceElementIter != root.element_begin("objects")->element_end(); ++extForceElementIter) {
            forceQueues_.emplace_back(new QueueForce());
        //    OpenSim::ExternalForceFromQueue<QueueForce>* ef = 
             extForces_.push_back(new OpenSim::ExternalForceFromQueue(*(forceQueues_.back()), *extForceElementIter));
        }
    }



    template<typename Q1, typename Q2, typename Q3, typename Q4, typename L>
    void QueuesToContactForces<Q1, Q2, Q3, Q4, L>::operator()() {

        //0. subscriptions to queues
        inputStateQueue_.subscribe();
        inputMuscleForcesQueue_.subscribe();





        //3. start the thread to move forces from Q2 to forceQueues_
   
        auto fut(std::async(std::launch::async, [=](){
            
            inputExternalForcesQueue_.subscribe();
            for (auto& l : latches_)
                l->wait();

            bool runCondition(true);
            do {
                cout << "Waiting for frame" << endl;
                QueueData<std::vector<ExternalForceData>> forces(inputExternalForcesQueue_.pop());
                if (!forces.data.empty()) {
                    std::cout << "Splitting forces\n";
                    for (size_t i(0); i < forces.data.size(); ++i) {
                        QueueData<ExternalForceData> currentForce{ forces.time, forces.data.at(i) };
                        forceQueues_.at(i)->push(currentForce);
                    }
                }
                else {
                    runCondition = false;
                    QueueData<ExternalForceData> eof;
                    eof.time = std::numeric_limits<double>::infinity();
                    for (auto& forceQueue : forceQueues_)
                        forceQueue->push(eof);
                }
            } while (runCondition);
        }));
   
        //2. call the latch, say that we are done with initialisations
        for (auto& l : latches_)
            l->wait();

        //1. add extForces to the model, hopefully I won't have problems with queues and opensim
        for (auto &force : extForces_)
            model_.addForce(force);

        JointContactFromOpensim jointContactModel(model_);
        
        //4. start the loop to synchronise everything and calculate JCF..



        
        fut.wait();
    }
}