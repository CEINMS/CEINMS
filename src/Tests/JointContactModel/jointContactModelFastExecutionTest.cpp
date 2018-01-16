#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/ContactModelXmlReader.h"
#include "ceinms/DataTable.h"
#include "ceinms/QueuesToIntersegmentalForces.h"
#include "ceinms/QueuesToJointReactionForces.h"
#include "ceinms/LoggerOnQueues.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/FileUtils.h"
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"
#include "ceinms/QueueToStorageFile.h"
#include <boost/filesystem.hpp>
using namespace ceinms;

#include <OpenSim/OpenSim.h>
using OpenSim::Storage;
using OpenSim::CoordinateActuator;
#include <OpenSim/Tools/InverseDynamicsTool.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;





class KriggGenericCF {

public:
    KriggGenericCF(const string& modelFilename, const string& kinematicsFilename, const string& externalLoadsXmlFilename) :
        modelFilename_(modelFilename), kinematicsFilename_(kinematicsFilename), externalLoadsXmlFilename_(externalLoadsXmlFilename)
    {}
    void setKinematics();
    void setExternalLoads();
    void trainModel();
    vector<double> getContactForces(const DataTable<double>& muscleForces);
private:
    void calculateIntersegmentalForces();
    string modelFilename_, kinematicsFilename_, externalLoadsXmlFilename_;
};


void KriggGenericCF::calculateIntersegmentalForces() {

    GeneralisedCoordinatesQueue generalisedCoordinateQueue;
    MultipleExternalForcesQueue multipleExternalForcesQueue;
    VecContactForcesQueue contactForcesQueue;
    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> subject;
    ceinms::Concurrency::Latch doneWithSubscriptions(4), doneWithExecution(4);
    OpenSim::Array<string> jointNamesOS;

    OpenSim::Model osimModel(modelFilename_);
    osimModel.getJointSet().getNames(jointNamesOS);
    std::vector<std::string> jointNames;
    ArrayConverter::toStdVector(jointNamesOS, jointNames);
    GeneralisedCoordinatesFromStorageFile generalisedCoordinatesFromStorageFile(
        generalisedCoordinateQueue,
        doneWithSubscriptions,
        doneWithExecution,
        modelFilename_,
        kinematicsFilename_,
        8);

    QueuesToIntersegmentalForces queuesToIntersegmentalForces(
        generalisedCoordinateQueue,
        multipleExternalForcesQueue,
        contactForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        subject,
        modelFilename_,
        externalLoadsXmlFilename_,
        jointNames
    );

    ExternalForcesFromStorageFile externalForcesFromStorageFile(
        multipleExternalForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        modelFilename_,
        externalLoadsXmlFilename_);

    QueueToFileLogger
    QueueToStorageFile<VecContactForcesData> logger(
        contactForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        jointNames,
        "ceinms_int_forces",
        outputDir);


    std::thread coordinateThr(std::ref(generalisedCoordinatesFromStorageFile));
    std::thread ifThread(std::ref(queuesToIntersegmentalForces));
    std::thread externalForcesThr(std::ref(externalForcesFromStorageFile));
    std::thread loggerThr(std::ref(logger));

    coordinateThr.join();
    ifThread.join();
    externalForcesThr.join();
    loggerThr.join();
}



int main() {

    // (i) implement OpenSim-less calculation of joint contact forces using a Krigging method

    return 0;
}