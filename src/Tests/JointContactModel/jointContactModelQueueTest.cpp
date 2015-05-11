#include "ceinms/QueuesToContactForces.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
using namespace ceinms;
using ceinms::Concurrency::Queue;
using ceinms::Concurrency::Latch;
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include <OpenSim/OpenSim.h>
#include <vector>
#include <memory>
#include "ceinms/ArrayConverter.h"
#include "ceinms/GeneralisedCoordinatesData.h"


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

    QueuesToContactForces::GeneralisedCoordinatesQueue generalisedCoordinatesQueue;
    QueuesToContactForces::ExternalForcesQueue externalForcesQueue;
    QueuesToContactForces::MuscleForcesQueue muscleForcesQueue;
    QueuesToContactForces::ContactForcesQueue outputContactForcesQueue;
    Latch latch(4);
    using MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    MyNMSmodel subject;

    string baseDir("C:/Users/S2849511/coding/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string osimFilename(baseDir + "model.osim");        
  
    QueuesToContactForces contactForcesEstimator(
        generalisedCoordinatesQueue,
        externalForcesQueue,
        muscleForcesQueue,
        outputContactForcesQueue,
        latch,
        subject,
        osimFilename,
        externalLoadsFilename,
        { "ankle_r" });

    //1 run state producer
    // init model to get the correct state structure
    OpenSim::Model model(osimFilename);
    model.initSystem();
    GeneralisedCoordinatesData generalisedCoordinates(model.getNumCoordinates());

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
        
        latch.wait();
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

            QueuesToContactForces::GeneralisedCoordinatesFrame generalisedCoordinatesFrame{ t, generalisedCoordinates };
            generalisedCoordinatesQueue.push(generalisedCoordinatesFrame);
        }
        QueuesToContactForces::GeneralisedCoordinatesFrame eof{ std::numeric_limits<double>::infinity(), 0 };
        generalisedCoordinatesQueue.push(eof);
    }));

    OpenSim::Storage externalForcesStorage(externalLoads.getDataFileName());
    OpenSim::Array<double> timeColumnExternalLoadsOS;
    vector<double> timeColumnExternalLoads;
    externalForcesStorage.getTimeColumn(timeColumnExternalLoadsOS);
    ceinms::ArrayConverter::toStdVector(timeColumnExternalLoadsOS, timeColumnExternalLoads);
    
    auto forcesThread(std::async(std::launch::async, [&](){

            latch.wait();
        for (auto t : timeColumnExternalLoads) {
            QueuesToContactForces::ExternalForcesFrame externalForcesFrame;
            externalForcesFrame.time = t;
            for (unsigned i(0); i < externalLoads.getSize(); ++i) {
                ExternalForceData currentExtForce;
                currentExtForce.setForceVector(externalLoads.get(i).getForceAtTime(t));
                currentExtForce.setForceApplicationPoint(externalLoads.get(i).getPointAtTime(t));
                currentExtForce.setTorque(externalLoads.get(i).getTorqueAtTime(t));
                externalForcesFrame.data.push_back(currentExtForce);
            }
            externalForcesQueue.push(externalForcesFrame);
        }
        QueuesToContactForces::ExternalForcesFrame eoForces;
        eoForces.time = std::numeric_limits<double>::infinity();
        externalForcesQueue.push(eoForces); //eof
    }));

    auto logThread(std::async(std::launch::async, [&](){

        outputContactForcesQueue.subscribe();
        latch.wait();
        double time(0);
        bool runCondition(true);
        while (runCondition) {
            auto contactForces(outputContactForcesQueue.pop());
            time = contactForces.time;
            if (time == std::numeric_limits<double>::infinity()) runCondition = false;
            if (runCondition)
                cout << contactForces.data << endl;
        }
    }));

    //launch thread for contact forces
    std::thread contactForcesEstimatorThread(std::ref(contactForcesEstimator));

    cout << "Waiting to join\n" << std::flush;
    contactForcesEstimatorThread.join();
    forcesThread.wait();
    statesThread.wait();
    logThread.wait();
    return 0;


}