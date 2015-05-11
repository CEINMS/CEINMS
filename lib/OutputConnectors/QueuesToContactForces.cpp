#include "ceinms/QueuesToContactForces.h"
#include "ceinms/JointContactFromOpensim.h"
#include <string>
#include <thread>
#include <limits>
#include <future>
#include <algorithm>

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

    ceinms::ExternalForceData ExternalForceFromQueue::getExternalForceDataFromPastValues(double time) const {

        auto it(std::lower_bound(pastExternalForceFrames_.begin(), pastExternalForceFrames_.end(), time, [](
            ceinms::QueueData<ceinms::ExternalForceData>& lhs,
            double t){ return lhs.time < t; }));
        return it->data;

    }


    ceinms::ExternalForceData ExternalForceFromQueue::getExternalForceDataFromQueue(double time) const {
        
        ExternalForceFrame forceFrame;
        while (time > lastTime_ && runCondition_){
            forceFrame = externalForceQueue_->pop();
            if (forceFrame.time == std::numeric_limits<double>::infinity()) {
                runCondition_ = false;
                forceFrame.data.setForceVector(SimTK::Vec3(0.));
                forceFrame.data.setForceApplicationPoint(SimTK::Vec3(0.));
                forceFrame.data.setTorque(SimTK::Vec3(0.));
            }
            if (runCondition_) {
                pastExternalForceFrames_.push_back(forceFrame);
                lastTime_ = forceFrame.time;
            }
        }
        return forceFrame.data;
    }


    void ExternalForceFromQueue::computeForce(
        const SimTK::State& state,
        SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
        SimTK::Vector& generalizedForces) const
    {
        if (runCondition_) {
            double time(state.getTime());
            ceinms::ExternalForceData forceData;
            if (time <= lastTime_)
                forceData = getExternalForceDataFromPastValues(time);
            else
                forceData = getExternalForceDataFromQueue(time);

            //it could be possible to chenge the application point of the force on the fly, but at the moment we just keep the definitions from the xml
            const SimbodyEngine& engine = getModel().getSimbodyEngine();
            SimTK::Vec3 transformedForce, transformedPosition, transformedTorque;
            engine.transform(state, *forceExpressedInBody_, forceData.getForce(), engine.getGroundBody(), transformedForce);
            engine.transformPosition(state, *pointExpressedInBody_, forceData.getApplicationPoint(), *appliedToBody_, transformedPosition);
            ExternalForce::applyForceToPoint(state, *appliedToBody_, transformedPosition, transformedForce, bodyForces);
            engine.transform(state, *forceExpressedInBody_, forceData.getTorque(), engine.getGroundBody(), transformedTorque);
            ExternalForce::applyTorque(state, *appliedToBody_, transformedTorque, bodyForces);
        }
    }
 

    void ExternalForceFromQueue::connectToModel(Model& model) {

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
    }



    template<typename Q1, typename Q2, typename Q3, typename Q4, typename L>
    void QueuesToContactForces<Q1, Q2, Q3, Q4, L>::operator()() {

        //0. subscriptions to queues
        inputStateQueue_.subscribe();
        inputMuscleForcesQueue_.subscribe();

        //we create an internal latch, so who is outside doesn't have to bother to increase the count..
        Latch forceSplitLatch(2);


        //1. Divide external forces in different queues
        auto forceDistributorThr(std::async(std::launch::async, [&](){
            
            inputExternalForcesQueue_.subscribe();
            forceSplitLatch.wait();

            bool runCondition(true);
            do {
                QueueData<std::vector<ExternalForceData>> forces(inputExternalForcesQueue_.pop());
                if (forces.time < std::numeric_limits<double>::infinity()) {
                    for (size_t i(0); i < forces.data.size(); ++i) {
                        QueueData<ExternalForceData> currentForce;
                        currentForce.time = forces.time;
                        currentForce.data = forces.data.at(i);
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
        for (auto& l : latches_)
            l->wait();
        forceSplitLatch.wait();

        bool runCondition(true);
        while (runCondition) {
            QueueData<GeneralisedCoordinates> state(inputStateQueue_.pop());

            if (state.time == std::numeric_limits<double>::infinity())
                runCondition = false;
            if (runCondition) {
                jointContactModel.setTime(state.time);
                jointContactModel.setQ(state.data.getQ());
                jointContactModel.setQd(state.data.getQd());
                jointContactModel.setQdd(state.data.getQdd());
                jointContactModel.calculateReactionForces(true);
                typename Q4::type contactForces;
                contactForces.time = state.time;
                for (auto& joint : existingJointsToLog) {
                    SimTK::Vec3 reactionForce;
                    jointContactModel.getReactionForceInG(joint, reactionForce);
                    contactForces.data.push_back(reactionForce);

              //      oF << state.time << "\t";
              //      oF << reactionForce[0] << "\t" <<
              //          reactionForce[1] << "\t" <<
              //          reactionForce[2] << "\t";
                }
                outputContactForcesQueue_.push(contactForces);
             //   oF << std::endl;
            }
        }
        typename Q4::type contactForces;
        contactForces.time = std::numeric_limits<double>::infinity();
        outputContactForcesQueue_.push(contactForces);

        forceDistributorThr.wait();
    }
}