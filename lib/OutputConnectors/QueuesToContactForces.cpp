#include "QueuesToContactForces.h"
#include "JointContactFromOpensim.h"
#include <string>
#include <thread>
#include <limits>
#include <future>
#include <algorithm>
#include <iostream> //
#include <ostream> //

namespace OpenSim {

    ExternalForceFromQueue::ExternalForceFromQueue
        (std::shared_ptr<ExternalForceQueue> externalForceQueue,
        const std::string& appliedToBodyName,
        const std::string& forceExpressedInBodyName,
        const std::string& pointExpressedInBodyName) :
        externalForceQueue_(externalForceQueue),
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr),
        lastTime_(std::numeric_limits<double>::lowest()),
        isSubscribed(false),
        runCondition_(true)
    {
        setAppliedToBodyName(appliedToBodyName);
        setForceExpressedInBodyName(forceExpressedInBodyName);
        setPointExpressedInBodyName(pointExpressedInBodyName);
        
    }


    ExternalForceFromQueue::ExternalForceFromQueue(std::shared_ptr<ExternalForceQueue> externalForceQueue, SimTK::Xml::Element& node) :
        externalForceQueue_(externalForceQueue),
        appliedToBody_(nullptr),
        forceExpressedInBody_(nullptr),
        pointExpressedInBody_(nullptr),
        lastTime_(std::numeric_limits<double>::lowest()),
        isSubscribed(false),
        runCondition_(true)
    {
        ExternalForce::updateFromXMLNode(node);
      
    }

    /*
    template<typename Q>
    void ExternalForceFromQueue<Q>::updateFromXMLNode(SimTK::Xml::Element& aNode, int versionNumber) {
        
        appliedToBodyName_ = aNode.element_begin("applied_to_body")->getValue();
        forceExpressedInBodyName_ = aNode.element_begin("force_expressed_in_body")->getValue();
        pointExpressedInBodyName_ = aNode.element_begin("point_expressed_in_body")->getValue();
            
    }
    */

    ceinms::ExternalForceData ExternalForceFromQueue::getForceDataFromPastValues(double time) const {

        auto it(std::lower_bound(pastForces_.begin(), pastForces_.end(), time, [](
            ceinms::QueueData<ceinms::ExternalForceData>& lhs,
            double t){ return lhs.time < t; }));
        std::cout << "from past valuse queue: " << *it << std::endl << std::flush;
   //     std::cout << "time from past values: " << it->time << std::endl << std::flush;
        return it->data;

    }


    ceinms::ExternalForceData ExternalForceFromQueue::getForceDataFromQueue(double time) const {

        
        ExternalForceQueue::type forceData;
        while (time > lastTime_ && runCondition_){
            forceData = externalForceQueue_->pop();
            if (forceData.time == std::numeric_limits<double>::infinity()) {
                runCondition_ = false;
                forceData.data.grf = SimTK::Vec3(0.);
                forceData.data.cop = SimTK::Vec3(0.);
                forceData.data.moments = SimTK::Vec3(0.);

            }
            if (runCondition_) {
                std::cout << "from force queue: " << forceData << std::endl << std::flush;

                pastForces_.push_back(forceData);
                lastTime_ = forceData.time;
            }
        }
        return forceData.data;
    }


    void ExternalForceFromQueue::computeForce(
        const SimTK::State& state,
        SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
        SimTK::Vector& generalizedForces) const
    {
        if (runCondition_) {
            //     std::cout << "compute force\n" << std::flush;
            double time(state.getTime());
            //    std::cout << "Time " << time << std::endl << std::flush;
            ceinms::ExternalForceData forceData;
            if (time <= lastTime_)
                forceData = getForceDataFromPastValues(time);
            else
                forceData = getForceDataFromQueue(time);


            const SimbodyEngine& engine = getModel().getSimbodyEngine();
            engine.transform(state, *forceExpressedInBody_, forceData.grf, engine.getGroundBody(), forceData.grf);
            engine.transformPosition(state, *pointExpressedInBody_, forceData.cop, *appliedToBody_, forceData.cop);
            ExternalForce::applyForceToPoint(state, *appliedToBody_, forceData.cop, forceData.grf, bodyForces);
            engine.transform(state, *forceExpressedInBody_, forceData.moments, engine.getGroundBody(), forceData.moments);
            ExternalForce::applyTorque(state, *appliedToBody_, forceData.moments, bodyForces);
        }
    }
 

    void ExternalForceFromQueue::connectToModel(Model& model) {

    //    std::cout << "Connecting to model\n" << std::flush;
        if (!isSubscribed) {
            externalForceQueue_->subscribe();
            isSubscribed = true;
        }
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
        modelFilename_(osimModelFilename),
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
            extForces_.push_back(new OpenSim::ExternalForceFromQueue(forceQueues_.back(), *extForceElementIter));
        }
        cout << "Number of external forces: " << extForces_.size() << std::endl << std::flush;
    }



    template<typename Q1, typename Q2, typename Q3, typename Q4, typename L>
    void QueuesToContactForces<Q1, Q2, Q3, Q4, L>::operator()() {

        //0. subscriptions to queues
        inputStateQueue_.subscribe();
        inputMuscleForcesQueue_.subscribe();

        //3. start the thread to move forces from Q2 to forceQueues_
   
        auto fut(std::async(std::launch::async, [&](){
            
            inputExternalForcesQueue_.subscribe();
            for (auto& l : latches_)
                l->wait();

            bool runCondition(true);
            do {
    //            cout << "Waiting for frame" << endl;
                QueueData<std::vector<ExternalForceData>> forces(inputExternalForcesQueue_.pop());
         //       cout << forces << endl;
                if (forces.time < std::numeric_limits<double>::infinity()) {
                    for (size_t i(0); i < forces.data.size(); ++i) {
                        QueueData<ExternalForceData> currentForce;
                        currentForce.time = forces.time;
                        currentForce.data = forces.data.at(i);
                        forceQueues_.at(i)->push(currentForce);
               //         std::cout << "Splitting forces\nTime: " << currentForce.time << std::endl << std::flush;

                    }

                }
                else {
                    runCondition = false;
                    QueueData<ExternalForceData> eof;
                    eof.time = std::numeric_limits<double>::infinity();
                    eof.data.id = 99;
                    for (auto& forceQueue : forceQueues_)
                        forceQueue->push(eof);
                }
            } while (runCondition);
        }));


 
        JointContactFromOpensim jointContactModel(modelFilename_, extForces_);
        vector<string> osimJointNames, existingJointsToLog;
        jointContactModel.getJointNamesFromOsimModel(osimJointNames);
        for (auto& jointName : jointsToLog_) {
            auto it(std::find(osimJointNames.begin(), osimJointNames.end(), jointName));
            if (it != osimJointNames.end())
                existingJointsToLog.emplace_back(*it);
            else
                cout << jointName << " does not exist." << endl;
        }
        //2. call the latch, say that we are done with initialisations
        
        std::ofstream oF("jrf.txt");

        for (auto& l : latches_)
            l->wait();


        bool runCondition(true);
        while (runCondition) {
            QueueData<GeneralisedCoordinates> state(inputStateQueue_.pop());
      //      std::cout << "Reading state from queue. Time: " << state.getTime() << std::endl << std::flush;

            if (state.time == std::numeric_limits<double>::infinity())
                runCondition = false;
            if (runCondition) {
                jointContactModel.setTime(state.time);
                jointContactModel.setQ(state.data.getQ());
                jointContactModel.setQd(state.data.getQd());
                jointContactModel.setQdd(state.data.getQdd());

                jointContactModel.calculateReactionForces(true);
                for (auto& joint : existingJointsToLog) {
                    SimTK::Vec3 reactionForce;
                    jointContactModel.getReactionForceInG(joint, reactionForce);
                    oF << state.time << "\t";
                    oF << reactionForce[0] << "\t" <<
                        reactionForce[1] << "\t" <<
                        reactionForce[2] << "\t";
                }
                oF << std::endl;
           //     cout << "reaction force " << reactionForce << endl;
            }
        }



        //4. start the loop to synchronise everything and calculate JCF..
       // model_.updVisualizer().show(state);



        
        fut.wait();
    }
}