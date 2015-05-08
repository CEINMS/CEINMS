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
    Queue<SimTK::State> stateQueue;
    Queue<ForcesData> externalForcesQueue;
    Queue<MuscleForcesData> muscleForcesQueue;
    Queue<ContactForcesData> outputContactForcesQueue;
    std::vector<std::unique_ptr<Latch>> latches;
    latches.emplace_back(new Latch(4));
    using MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    MyNMSmodel subject;

    string baseDir("C:/Users/S2849511/coding/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string osimFilename(baseDir + "model.osim");        
  
    
    //  std::string osimFilename("osimFilename");
 //   std::string externalLoadsXmlFilename("externalLoadsXmlFilename");
    
  
   // std::string kinematicsFilename("kinematics.mot");



    QueuesToContactForces<
        Queue<SimTK::State>,
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
        { "ankle_angle_l" });

    //1 run state producer
    // init model to get the correct state structure
    OpenSim::Model model(osimFilename);
    SimTK::State state(model.initSystem());

    OpenSim::ExternalLoads externalLoads(model, externalLoadsFilename);
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
            state.setTime(t);
            OpenSim::Array<double> q;
            kinematicsSplines.evaluate(q, 0, t);
            for (size_t i(0); i < model.getNumCoordinates(); ++i)
                state.updQ()[i] = q[i];

            OpenSim::Array<double> qd;
            kinematicsSplines.evaluate(qd, 1, t);
            for (size_t i(0); i < model.getNumCoordinates(); ++i)
                state.updQDot()[i] = qd[i];

            OpenSim::Array<double> qdd;
            kinematicsSplines.evaluate(qdd, 2, t);
            for (size_t i(0); i < model.getNumCoordinates(); ++i)
                state.updQDotDot()[i] = qdd[i];

            //    cout << "Pushing state on queue\n";
            stateQueue.push(state);
        }

        state.setTime(std::numeric_limits<double>::infinity());  //eof
        stateQueue.push(state);
    }));

//    cout << stateQueue;
    // push external forces to queue
  //  OpenSim::ExternalLoads externalLoads(model, externalLoadsXmlFilename);


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
        //        cout << "Pushing external force on queue "<<
         //           "Time " << t << endl << "values: " 
          //          << currentExtForce << endl << std::flush;
            }
           
            externalForcesQueue.push(forces);
       //     cout << "externalForcesQueue " << externalForcesQueue << std::flush;
        }
        ForcesData eoForces;
        eoForces.time = std::numeric_limits<double>::infinity();
        externalForcesQueue.push(eoForces); //eof
    }));
    //cout << "externalForcesQueue " << externalForcesQueue;


    //launch thread for contact forces
    std::thread contactForcesEstimatorThread(std::ref(contactForcesEstimator));
//    auto fut(std::async(std::launch::async, std::ref(contactForcesEstimator)));

    
 //   for (auto& l : latches)
 //       l->wait();
    cout << "Waiting to join\n" << std::flush;
    contactForcesEstimatorThread.join();
    forcesThread.wait();
    statesThread.wait();
    return 0;


}