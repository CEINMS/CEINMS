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
#include "ceinms/QueueToDataTable.h"
#include "ceinms/BatchOpenSimEvaluator.h"
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

#include <boost/filesystem.hpp>
using boost::filesystem::absolute;
using boost::filesystem::path;
using boost::filesystem::canonical;


class KriggGenericCF {

public:
    KriggGenericCF(const string& modelFilename, const string& kinematicsFilename, const string& externalLoadsXmlFilename);
    DataTable<SimTK::Vec3> getReactionForces(const DataTable<double>& muscleForces) const;
    DataTable<SimTK::Vec3> getIntersegmentalForces() const {
        return intersegmentalForces_;
    };
    vector<string> getMuscleNames() const;
    size_t getMusclesSize() const {
        return muscleNames_.size();
    }

    size_t getFrameNumber() const {
        return intersegmentalForces_.getNRows();
    }


private:
    void calculateIntersegmentalForces();
    void calculateUnitaryContributionOfMuscleToContactForces();
    OpenSim::Model model_;
    int nForces_, nDofs_, nFrames_;
    vector<double> timeColumm_;
    vector<string> muscleNames_, dofNames_;
    BatchOpenSimEvaluator evaluator_;
    DataTable<SimTK::Vec3> intersegmentalForces_;
    std::map<string, DataTable<SimTK::Vec3>> unitaryContributionOfMuscleToContactForces_;

};

KriggGenericCF::KriggGenericCF(const string& modelFilename, const string& kinematicsFilename, const string& externalLoadsXmlFilename) :
    model_(modelFilename)
{
    nForces_ = model_.getForceSet().getSize();
    nDofs_ = model_.getCoordinateSet().getSize();
    OpenSim::Storage kinematicsData(kinematicsFilename);
    OpenSim::Array<double> tempTimeCol;
    kinematicsData.getTimeColumn(tempTimeCol);
    timeColumm_ = ArrayConverter::toStdVector(tempTimeCol);
    TrialData trialData;
    trialData.setOpenSimData(modelFilename, externalLoadsXmlFilename, kinematicsFilename);
    nFrames_ = trialData.getQ().getNRows();
    vector<TrialData> osimTrialData{ trialData };
    evaluator_.setOpenSimTrialData(osimTrialData);
    
    OpenSim::Array<string> muscleNames;
    model_.getForceSet().getNames(muscleNames);
    muscleNames_ = ArrayConverter::toStdVector(muscleNames);

    OpenSim::Array<string> dofNames;
    model_.getCoordinateSet().getNames(dofNames);
    dofNames_ = ArrayConverter::toStdVector(dofNames);

    calculateIntersegmentalForces();
    calculateUnitaryContributionOfMuscleToContactForces();
}

void KriggGenericCF::calculateIntersegmentalForces() {

    //forces will be at 0 by default
    Result result(nForces_, nDofs_, nFrames_, nFrames_);
    result.setMuscleNames(muscleNames_);
    result.setDofNames(dofNames_);
    for (int i(0); i < timeColumm_.size(); ++i)
        result.forces.setTime(timeColumm_[i], i);
    vector<Result> results{ result };
    evaluator_.evaluate(results);
    intersegmentalForces_ = evaluator_.getResults()[0].jointReactionForces;
}

void KriggGenericCF::calculateUnitaryContributionOfMuscleToContactForces() {

    Result result(nForces_, nDofs_, nFrames_, nFrames_);
    result.setMuscleNames(muscleNames_);
    result.setDofNames(dofNames_);
    vector<Result> results{ result };
    vector<double> empty(nForces_, 0.);
    for (int i(0); i < muscleNames_.size(); ++i) {
        vector<double> unitForce(empty);
        unitForce[i] = 1.;
        for (int r(0); r < nFrames_; ++r) {
            results[0].forces.setRow(r, unitForce);
        }
        evaluator_.evaluate(results);
        unitaryContributionOfMuscleToContactForces_.insert(
            std::make_pair(muscleNames_[i], evaluator_.getResults()[0].jointReactionForces - intersegmentalForces_)
        );
    }
}


DataTable<SimTK::Vec3> KriggGenericCF::getReactionForces(const DataTable<double>& muscleForces) const {

    DataTable<SimTK::Vec3> reactionForces = intersegmentalForces_;
    for (int i(0); i < muscleNames_.size(); ++i) {
        auto column(muscleForces.getColumn(i));
        const DataTable<SimTK::Vec3>& unitContribution = unitaryContributionOfMuscleToContactForces_.at(muscleNames_[i]);
        for (int r(0); r < column.size(); ++r) {
            for (int c(0); c < unitContribution.getNColumns(); ++c) {
                reactionForces.at(r,c) += unitContribution.at(r, c) * column[i];
            }
        }
    }
    return reactionForces;
}


vector<string> KriggGenericCF::getMuscleNames() const {
    return muscleNames_;
}

//this is already done internally in JointContactFromOpensim, so we just do it for the test
void addCoordinateActuators(OpenSim::Model& model, double control, bool removeMuscles) {

    const OpenSim::CoordinateSet& cs = model.getCoordinateSet();
    if (removeMuscles)
        model.updForceSet().setSize(0); //remove all actuators from the model
    for (unsigned i(0); i < model.getNumCoordinates(); ++i) {
        CoordinateActuator *actuator = new CoordinateActuator();
        actuator->setCoordinate(&cs.get(i));
        string actuatorName(cs.get(i).getName() + "_actuator");
        actuator->setName(actuatorName);
        actuator->setOptimalForce(1);
        actuator->setMaxControl(control);
        actuator->setMinControl(-control);
        model.addForce(actuator);
    }
}

void calculateIntersegmentaForcesWithOpensim(std::string modelFilename, const OpenSim::Storage& kinematics, const string& externalLoadsFilename, const vector<double>& timeColumn, std::string outputDir) {

    OpenSim::Model model(modelFilename);
    double firstTime = timeColumn.front();
    double lastTime = timeColumn.back();
    addCoordinateActuators(model, 100000, true);
    model.print(outputDir + "/musclelessModel.osim");
    OpenSim::Model modelJR = model;
    SimTK::State state = model.initSystem();

    auto setupAnalTool([&](OpenSim::AnalyzeTool& analTool, const SimTK::State& modelState) {
        //      analTool.setCoordinatesFileName(kinematicsFilename);
        path p(externalLoadsFilename);
        path baseDir(p.parent_path());
        OpenSim::ExternalLoads externalLoads(model, externalLoadsFilename);
        auto forcesFilename = externalLoads.getDataFileName();
        if (!FileUtils::isAbsolute(forcesFilename.c_str())) {
            forcesFilename = (baseDir / forcesFilename).string();
            externalLoads.setDataFileName(forcesFilename);
        }
        auto ikFilename = externalLoads.getExternalLoadsModelKinematicsFileName();
        if (!FileUtils::isAbsolute(ikFilename.c_str())) {
            ikFilename = (baseDir / ikFilename).string();
            externalLoads.setExternalLoadsModelKinematicsFileName(ikFilename);
        }
   //     std::string correctExtLoads((baseDir / "fixedExtLoads.xml").string());
   //     externalLoads.print(correctExtLoads);
   //     analTool.setExternalLoadsFileName(correctExtLoads);
        analTool.setStatesFromMotion(modelState, kinematics, true);
        analTool.updExternalLoads().setDataFileName(forcesFilename);
        analTool.setStartTime(firstTime);
        analTool.setFinalTime(lastTime);
        analTool.setPrintResultFiles(true);
        analTool.setSolveForEquilibrium(false);
        analTool.setLowpassCutoffFrequency(8);
        analTool.setReplaceForceSet(true);
    });

    //static opt
    OpenSim::StaticOptimization* sopt = new OpenSim::StaticOptimization(&model);
    model.addAnalysis(sopt);
    OpenSim::AnalyzeTool analiseToolForSO(model);
    setupAnalTool(analiseToolForSO, state);
    analiseToolForSO.run();
    sopt->printResults("", outputDir);
    model.removeAnalysis(sopt, false);

    //joint reaction
    SimTK::State stateJR = modelJR.initSystem();
    OpenSim::JointReaction* jreact = new OpenSim::JointReaction(&modelJR);
    modelJR.addAnalysis(jreact);
    OpenSim::Array<string> jointNames;
    modelJR.getJointSet().getNames(jointNames);
    jreact->setForcesFileName("_StaticOptimization_force.sto");
    jreact->setJointNames(jointNames);
    jreact->print("JRsetup.xml");
    jreact->setName("JointReaction");
    OpenSim::AnalyzeTool analiseToolForJR(modelJR);
    setupAnalTool(analiseToolForJR, stateJR);
    analiseToolForJR.run();
    jreact->printResults("", outputDir);
}

void calculateIntersegmentaForcesWithCEINMS(
    std::string osimModelFilename, 
    std::string kinematicsStorageFilename, 
    std::string externalLoadsXmlFilename, 
    std::string outputDir) {

    GeneralisedCoordinatesQueue generalisedCoordinateQueue;
    MultipleExternalForcesQueue multipleExternalForcesQueue;
    VecContactForcesQueue contactForcesQueue;
    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> subject;
    ceinms::Concurrency::Latch doneWithSubscriptions(4), doneWithExecution(4);

    OpenSim::Model model(osimModelFilename);
    OpenSim::Array<string> jointNamesOS;
    model.getJointSet().getNames(jointNamesOS);
    std::vector<std::string> jointNames;
    ArrayConverter::toStdVector(jointNamesOS, jointNames);
    GeneralisedCoordinatesFromStorageFile generalisedCoordinatesFromStorageFile(
        generalisedCoordinateQueue,
        doneWithSubscriptions,
        doneWithExecution,
        osimModelFilename,
        kinematicsStorageFilename,
        8);

    QueuesToIntersegmentalForces queuesToIntersegmentalForces(
        generalisedCoordinateQueue,
        multipleExternalForcesQueue,
        contactForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        subject,
        osimModelFilename,
        externalLoadsXmlFilename,
        jointNames
    );

    ExternalForcesFromStorageFile externalForcesFromStorageFile(
        multipleExternalForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        externalLoadsXmlFilename);

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


template<typename T>
class DataTableToQueue {
public:
    DataTableToQueue(
        ceinms::Concurrency::Queue<QueueData<std::vector<T>>>& outputQueue,
        ceinms::Concurrency::Latch& doneWithSubscriptions,
        ceinms::Concurrency::Latch& doneWithExecution,
        const DataTable<T>& dataTable) :
        outputQueue_(outputQueue),
        doneWithSubscriptions_(doneWithSubscriptions),
        doneWithExecution_(doneWithExecution),
        dataTable_(dataTable) {}
        
    void operator()() {

        doneWithSubscriptions_.wait();
        for (int i(0); i < dataTable_.getNRows(); ++i) {
            QueueData<std::vector<T>> frame;
            frame.time = dataTable_.getTime(i);
            frame.data = dataTable_.getRow(i);
            outputQueue_.push(frame);
        }
        outputQueue_.push(EndOfData::get<std::vector<T>>());
        doneWithExecution_.wait();
    }

private:

    ceinms::Concurrency::Queue<QueueData<T>>& outputQueue_;
    ceinms::Concurrency::Latch& doneWithSubscriptions_;
    ceinms::Concurrency::Latch& doneWithExecution_;
    DataTable<T> dataTable_;


};

void calculateReactionForcesWithCEINMS(
    std::string osimModelFilename,
    std::string kinematicsStorageFilename,
    std::string externalLoadsXmlFilename,
    std::string outputDir) {

    GeneralisedCoordinatesQueue generalisedCoordinateQueue;
    MultipleExternalForcesQueue multipleExternalForcesQueue;
    MuscleForcesQueue muscleForcesQueue;
    VecContactForcesQueue contactForcesQueue;
    EstimatedTorquesQueue estimatedTorquesQueue;
    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> subject;
    ceinms::Concurrency::Latch doneWithSubscriptions(4), doneWithExecution(4);

    OpenSim::Model model(osimModelFilename);
    OpenSim::Array<string> jointNamesOS;
    model.getJointSet().getNames(jointNamesOS);
    std::vector<std::string> jointNames;
    ArrayConverter::toStdVector(jointNamesOS, jointNames);
    GeneralisedCoordinatesFromStorageFile generalisedCoordinatesFromStorageFile(
        generalisedCoordinateQueue,
        doneWithSubscriptions,
        doneWithExecution,
        osimModelFilename,
        kinematicsStorageFilename,
        8);

    ExternalForcesFromStorageFile externalForcesFromStorageFile(
        multipleExternalForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        externalLoadsXmlFilename);


    QueuesToJointReactionForces queuesToJointReactionForces(
        generalisedCoordinateQueue,
        multipleExternalForcesQueue,
        muscleForcesQueue,
        estimatedTorquesQueue,
        contactForcesQueue,
        doneWithSubscriptions,
        doneWithExecution,
        subject,
        osimModelFilename,
        externalLoadsXmlFilename,
        jointNames
    );



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

template<typename T>
void printDataTable(const DataTable<T>& table, const string& outputDir, const string& filename) {

    FileLogger<vector<T>> logger(table.getLabels(), filename, outputDir);
    for (size_t r(0); r < table.getNRows(); ++r) {
        QueueData<vector<T>> data;
        data.time = table.getTime(r);
        data.data = table.getRow(r);
        logger.logFrame(data);
    }
    logger.print();
}


int main() {

    path dataPath(path(FileUtils::simplify(BASE_DIR)) / "Data");
    path inputPath(dataPath / "input");
    path outputPath(dataPath / "output");
    cout << "Data path " << dataPath << endl;
    string outputDir(outputPath.string());
    string osimModelFilename((inputPath / "model.osim").string());
    string kinematicsStorageFilename((inputPath / "ik.mot").string());
    string externalLoadsXmlFilename((inputPath / "ExtForcesSetup.xml").string());
    
    KriggGenericCF krigg(osimModelFilename, kinematicsStorageFilename, externalLoadsXmlFilename);
    cout << "Number of muscles in Krigg model: " << krigg.getMusclesSize() << endl;
    cout << "Number of frames in Krigg model: " << krigg.getFrameNumber() << endl;
    DataTable<double> muscleForces(krigg.getFrameNumber(), krigg.getMusclesSize());
    muscleForces.setLabels(krigg.getMuscleNames());

    //calculate intersegmental forces
    //auto intersegmentalForces(krigg.getReactionForces(muscleForces));
    OpenSim::Model model(osimModelFilename);
    auto intersegmentalForces(krigg.getIntersegmentalForces());
    printDataTable(intersegmentalForces, outputDir, "krigg_intersegmental.sto");
    DataTable<double> emptyTable(intersegmentalForces.getNRows(), model.getMuscles().getSize());

    auto intersegmentalForces2(krigg.getReactionForces(emptyTable));
    printDataTable(intersegmentalForces2, outputDir, "krigg_intersegmental2.sto");
  
    calculateIntersegmentaForcesWithCEINMS(
        osimModelFilename,
        kinematicsStorageFilename,
        externalLoadsXmlFilename,
        outputDir);


  /*  OpenSim::Storage originalKinematics(kinematicsStorageFilename);
    OpenSim::Array<double> tempTimeCol;
    originalKinematics.getTimeColumn(tempTimeCol);
    vector<double> timeColumn = ArrayConverter::toStdVector(tempTimeCol);
    calculateIntersegmentaForcesWithOpensim(osimModelFilename, kinematicsStorageFilename, externalLoadsXmlFilename, timeColumn, outputPath.string());
    */
    return 0;
}