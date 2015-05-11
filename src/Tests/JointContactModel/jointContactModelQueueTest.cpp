#include "QueuesToContactForces.h"
#include "Queue.h"
#include "Latch.h"
using namespace ceinms;
using ceinms::Concurrency::Queue;
using ceinms::Concurrency::Latch;
#include "NMSmodel.h"
#include "Activation/ExponentialActivation.h"
#include "Tendon/ElasticTendon_BiSec.h"
#include "Curve.h"
#include <OpenSim/OpenSim.h>
#include <vector>
#include <memory>
#include "ArrayConverter.h"


OpenSim::GCVSplineSet getKinematicsSplines(OpenSim::Model& model, OpenSim::Storage& kinematics, double fc) {

    if (fc > 0) {
        kinematics.pad(kinematics.getSize() / 2.);
        kinematics.lowpassIIR(fc);
    }
    if (kinematics.isInDegrees()){
        model.getSimbodyEngine().convertDegreesToRadians(kinematics);
    }
    // Create differentiable splines of the coordinate data
    OpenSim::GCVSplineSet kinematicsSplines(5, &kinematics);

    //Functions must correspond to model coordinates and their order for the solver
    OpenSim::Array<string> coordinateNamesOS;
    model.getCoordinateSet().getNames(coordinateNamesOS);
    for (size_t i(0); i < coordinateNamesOS.getSize(); ++i){
        if (kinematicsSplines.contains(coordinateNamesOS.get(i))){
            kinematicsSplines.insert(i, kinematicsSplines.get(coordinateNamesOS.get(i)));
        }
        else{
            kinematicsSplines.insert(i, OpenSim::Constant(model.getCoordinateSet().get(i).getDefaultValue()));
        }
    }
    if (kinematicsSplines.getSize() > coordinateNamesOS.getSize()){
        kinematicsSplines.setSize(coordinateNamesOS.getSize());
    }

    return kinematicsSplines;
}

int main() {
    using MuscleForcesData = QueueData < std::vector<double> > ;
    using ContactForcesData = QueueData < std::vector<double> >;
    using ForcesData = QueueData < std::vector<ExternalForceData> > ;
    using StateData = QueueData < GeneralisedCoordinates > ;
    Queue<StateData> stateQueue;
    Queue<ForcesData> externalForcesQueue;
    Queue<MuscleForcesData> muscleForcesQueue;
    Queue<ContactForcesData> outputContactForcesQueue;
    std::vector<std::unique_ptr<Latch>> latches;
    latches.emplace_back(new Latch(3)); // the latch set to 3 is for: stateQueue, externalForcesQueue, contactForcesEstimator
    using MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    MyNMSmodel subject;

    string baseDir("C:/Users/S2849511/coding/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string osimFilename(baseDir + "model.osim");        
  
    QueuesToContactForces<
        Queue<StateData>,
        Queue<ForcesData>,
        Queue<MuscleForcesData>,
        Queue<ContactForcesData>,
        Latch> contactForcesEstimator(
        stateQueue,
        externalForcesQueue,
        muscleForcesQueue,
        outputContactForcesQueue,
        latches,
        subject,
        osimFilename,
        externalLoadsFilename,
        { "ankle_r" });

    //1 run state producer
    // init model to get the correct state structure
    OpenSim::Model model(osimFilename);
    model.initSystem();
    GeneralisedCoordinates generalisedCoordinates(model.getNumCoordinates());

    OpenSim::ExternalLoads externalLoads(model, externalLoadsFilename);
    externalLoads.invokeConnectToModel(model);
    string kinematicsFilename(externalLoads.getExternalLoadsModelKinematicsFileName());
    OpenSim::Storage kinematics(kinematicsFilename);
    double firstTime(kinematics.getFirstTime());
    double lastTime(kinematics.getLastTime());
    OpenSim::Array<double> timeColumnOS;
    kinematics.getTimeColumn(timeColumnOS);
    vector<double> timeColumn;
    ceinms::ArrayConverter::toStdVector(timeColumnOS, timeColumn);
    auto kinematicsSplines(getKinematicsSplines(model, kinematics, 8));

    auto statesThread(std::async(std::launch::async, [&](){

        for (auto& l : latches)
            l->wait();
        
        //push states into queue
        for (auto t : timeColumn) {
           
            OpenSim::Array<double> q;
            vector<double> stdQ;
            kinematicsSplines.evaluate(q, 0, t);
            ArrayConverter::toStdVector(q, stdQ);
            generalisedCoordinates.setQ(stdQ);

            OpenSim::Array<double> qd;
            vector<double> stdQd;
            kinematicsSplines.evaluate(qd, 1, t);
            ArrayConverter::toStdVector(qd, stdQd);
            generalisedCoordinates.setQd(stdQd);

            OpenSim::Array<double> qdd;
            vector<double> stdQdd;
            kinematicsSplines.evaluate(qdd, 2, t);
            ArrayConverter::toStdVector(qdd, stdQdd);
            generalisedCoordinates.setQdd(stdQdd);

            StateData state{ t, generalisedCoordinates };
            stateQueue.push(state);
        }
        StateData state{ std::numeric_limits<double>::infinity(), 0 };
        stateQueue.push(state);
    }));

    OpenSim::Storage externalForcesStorage(externalLoads.getDataFileName());
    OpenSim::Array<double> timeColumnExternalLoadsOS;
    vector<double> timeColumnExternalLoads;
    externalForcesStorage.getTimeColumn(timeColumnExternalLoadsOS);
    ceinms::ArrayConverter::toStdVector(timeColumnExternalLoadsOS, timeColumnExternalLoads);
    
    auto forcesThread(std::async(std::launch::async, [&](){

        for (auto& l : latches)
            l->wait();
        for (auto t : timeColumnExternalLoads) {
            ForcesData forces;
            forces.time = t;
            for (unsigned i(0); i < externalLoads.getSize(); ++i) {
                ExternalForceData currentExtForce;
                currentExtForce.grf = externalLoads.get(i).getForceAtTime(t);
                currentExtForce.cop = externalLoads.get(i).getPointAtTime(t);
                currentExtForce.moments = externalLoads.get(i).getTorqueAtTime(t);
                forces.data.push_back(currentExtForce);
            }
            externalForcesQueue.push(forces);
        }
        ForcesData eoForces;
        eoForces.time = std::numeric_limits<double>::infinity();
        externalForcesQueue.push(eoForces); //eof
    }));

    //launch thread for contact forces
    std::thread contactForcesEstimatorThread(std::ref(contactForcesEstimator));

    cout << "Waiting to join\n" << std::flush;
    contactForcesEstimatorThread.join();
    forcesThread.wait();
    statesThread.wait();
    return 0;


}