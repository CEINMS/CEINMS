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

int main() {
    using MuscleForcesData = QueueData < std::vector<double> > ;
    using ContactForcesData = QueueData < std::vector<double> >;

    Queue<SimTK::State> stateQueue;
    Queue<QueueData<ExternalForceData>> externalForcesQueue;
    Queue<MuscleForcesData> muscleForcesQueue;
    Queue<ContactForcesData> outputContactForcesQueue;
    std::vector<std::unique_ptr<Latch>> latches;
    latches.emplace_back(new Latch(3));
    latches.emplace_back(new Latch(2));
    using MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    MyNMSmodel subject;
    std::string osimFilename("osimFilename");
    std::string externalLoadsXmlFilename("externalLoadsXmlFilename");

   
    QueuesToContactForces<
        Queue<SimTK::State>,
        Queue<QueueData<ExternalForceData>>,
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
        externalLoadsXmlFilename,
        { "ankle_angle_l" });



    return 0;


}