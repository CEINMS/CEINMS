#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/ContactModelXmlReader.h"
#include "ceinms/DataTable.h"
#include "ceinms/QueuesToIntersegmentalForces.h"
#include "ceinms/QueuesToJointReactionForces.h"
#include "ceinms/LoggerOnQueues.h"
#include "rtb/concurrency/Concurrency.h"
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
#include <random>

#include <boost/filesystem.hpp>
using boost::filesystem::absolute;
using boost::filesystem::path;
using boost::filesystem::canonical;

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


class KriggGenericCF {
    /* TODO: fix moment arm calculation, remove unused functions, calculate, test reaction analysis*/
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

    std::vector<double> getTimeColumn() const {
        return timeColumm_;
    }

private:
    void calculateMomentArms();
    void calculateIntersegmentalForces();
    void calculateUnitaryContributionOfMuscleToContactForces();
    OpenSim::Model model_;
    int nForces_, nDofs_, nFrames_;
    vector<double> timeColumm_;
    vector<string> muscleNames_, dofNames_;
    BatchOpenSimEvaluator evaluator_;
    DataTable<SimTK::Vec3> intersegmentalForces_;
    std::map<string, DataTable<SimTK::Vec3>> unitaryContributionOfMuscleToContactForces_;
    OpenSim::Storage kinematicsData_;
    vector<DataTable<double>> maData_;
};

KriggGenericCF::KriggGenericCF(const string& modelFilename, const string& kinematicsFilename, const string& externalLoadsXmlFilename) :
    model_(modelFilename), kinematicsData_(kinematicsFilename)
{

    nForces_ = model_.getForceSet().getSize();
    nDofs_ = model_.getCoordinateSet().getSize();
    OpenSim::Array<double> tempTimeCol;
    kinematicsData_.getTimeColumn(tempTimeCol);
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

    // This can be used to conditionally use external moment arm data
    calculateMomentArms();
    calculateIntersegmentalForces();
    calculateUnitaryContributionOfMuscleToContactForces();
}


void KriggGenericCF::calculateMomentArms() {
   
    OpenSim::Array<std::string> muscles;
    model_.getMuscles().getNames(muscles);
    std::unique_ptr<OpenSim::MuscleAnalysis> muscleTendonLengthAnalysis(new OpenSim::MuscleAnalysis(&model_));
    muscleTendonLengthAnalysis->setMuscles(muscles);
    muscleTendonLengthAnalysis->setStartTime(timeColumm_.front());
    muscleTendonLengthAnalysis->setEndTime(timeColumm_.back());
    muscleTendonLengthAnalysis->setComputeMoments(true);
    muscleTendonLengthAnalysis->setPrintResultFiles(false);

    //add analysis to model and update the model state
    model_.addAnalysis(muscleTendonLengthAnalysis.get());
    SimTK::State& stateMuscleAnalysis = model_.initSystem();
    model_.getMultibodySystem().realize(stateMuscleAnalysis, SimTK::Stage::Position);

    //create an AnalyzeTool with a passed in OpenSim Model (a model with the muscle analysis)
    OpenSim::AnalyzeTool musclePlotterTool(model_);
    //create a storage from a motion file
    //load motion file into the AnalyzeTool
    musclePlotterTool.setStatesFromMotion(stateMuscleAnalysis, kinematicsData_, true);

    //add analysis to the model and update model's state
    musclePlotterTool.addAnalysisSetToModel();
    stateMuscleAnalysis = model_.initSystem();
    model_.getMultibodySystem().realize(stateMuscleAnalysis, SimTK::Stage::Position);
    musclePlotterTool.setInitialTime(timeColumm_.front());
    musclePlotterTool.setFinalTime(timeColumm_.back());
    musclePlotterTool.setPrintResultFiles(false);
    //musclePlotterTool.setResultsDir("MuscleAnalysis");
    musclePlotterTool.setStatesFileName("MuscleAnalysis/test");
    musclePlotterTool.run();

    //retrieve ma values
    const OpenSim::ArrayPtrs<OpenSim::MuscleAnalysis::StorageCoordinatePair>& myMaArraysOfStorage = muscleTendonLengthAnalysis->getMomentArmStorageArray();
    size_t numCoordinates = myMaArraysOfStorage.getSize();
    for (size_t coordinateIterator = 0; coordinateIterator < numCoordinates; ++coordinateIterator) {

        string currentCoordinateName = myMaArraysOfStorage[coordinateIterator]->q->getName();
        auto it = find(dofNames_.begin(), dofNames_.end(), currentCoordinateName);
        if (it != dofNames_.end()) {
            DataTable<double> currentCoordinateMomentArm(timeColumm_.size(), muscleNames_.size());
            currentCoordinateMomentArm.setLabels(muscleNames_);
            for (size_t iCol = 0; iCol < muscleNames_.size(); ++iCol) {
                OpenSim::Array<double> tempCol;
                myMaArraysOfStorage[coordinateIterator]->momentArmStore->getDataColumn(iCol, tempCol);
                vector<double> column;
                ArrayConverter::toStdVector(tempCol, column);
                currentCoordinateMomentArm.setColumn(iCol, column);
            }
            maData_.push_back(currentCoordinateMomentArm);
        }
    }
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
    vector<double> empty(nForces_, 0.);
    for (int i(0); i < muscleNames_.size(); ++i) {
        vector<double> unitForce(empty);
        unitForce[i] = 1.;
        for (int r(0); r < nFrames_; ++r) {
            result.forces.setRow(r, unitForce);
        }
        result.forces.setTimeColumn(timeColumm_);
        for (int dofIdx(0); dofIdx < dofNames_.size(); ++ dofIdx) {
            vector<double> currentJointMoment = (maData_.at(dofIdx)* result.forces).accumulateColumns();
            result.torques.setColumn(dofIdx, currentJointMoment);
        }
        result.torques.setTimeColumn(timeColumm_);
        evaluator_.evaluate({ result });
        unitaryContributionOfMuscleToContactForces_.insert(
            std::make_pair(muscleNames_[i], evaluator_.getResults()[0].jointReactionForces - intersegmentalForces_)
        );

        path dataPath(path(FileUtils::simplify(BASE_DIR)) / "Data");
        path outputPath(dataPath / "output");
        printDataTable(unitaryContributionOfMuscleToContactForces_[muscleNames_[i]], outputPath.string(), "contribution_" + muscleNames_[i] + ".csv");
    }
}


DataTable<SimTK::Vec3> KriggGenericCF::getReactionForces(const DataTable<double>& muscleForces) const {

    DataTable<SimTK::Vec3> reactionForces = intersegmentalForces_;
    for (int i(0); i < muscleNames_.size(); ++i) {
        auto column(muscleForces.getColumn(i));
        const DataTable<SimTK::Vec3>& unitContribution = unitaryContributionOfMuscleToContactForces_.at(muscleNames_[i]);
        for (int r(0); r < column.size(); ++r) {
            for (int c(0); c < unitContribution.getNColumns(); ++c) {
                reactionForces.at(r,c) = reactionForces.at(r, c) + unitContribution.at(r, c) * column[r];
            }
        }
    }
    return reactionForces;
}


vector<string> KriggGenericCF::getMuscleNames() const {
    return muscleNames_;
}

//this is already done internally in JointContactFromOpensim, so we just do it for the test
void addCoordinateActuators(OpenSim::Model& model, double control, double optForce, bool removeMuscles, 
    const std::vector<string>& coordNameList) {

    OpenSim::CoordinateSet& cs = model.updCoordinateSet();
    if (removeMuscles)
        model.updForceSet().setSize(0); //remove all actuators from the model

    for (unsigned i(0); i < coordNameList.size(); ++i) {
 //       if (cs.get(i).getMotionType() == OpenSim::Coordinate::MotionType::Rotational) {
            CoordinateActuator *actuator = new CoordinateActuator();
            actuator->setCoordinate(&cs.get(coordNameList[i]));
            string actuatorName(cs.get(i).getName() + "_actuator");
            actuator->setName(actuatorName);
            actuator->setOptimalForce(optForce);
            actuator->setMaxControl(control);
            actuator->setMinControl(-control);
            model.addForce(actuator);
   //     }
    }
}

void calculateIntersegmentaForcesWithOpensim(
    std::string modelFilename, 
    const OpenSim::Storage& kinematics, 
    const string& externalLoadsFilename, 
    std::string outputDir) {

    OpenSim::Model model(modelFilename);
    double firstTime = kinematics.getFirstTime();
    double lastTime = kinematics.getLastTime();
    OpenSim::Array<string> coordsOS;
    model.getCoordinateSet().getNames(coordsOS);
    std::vector<string> coords = ArrayConverter::toStdVector(coordsOS);
 //   std::vector<string> coordsToRemove{ "pelvis_tilt", "pelvis_list", "pelvis_rotation", "pelvis_tx", "pelvis_ty", "pelvis_tz" };
    std::vector<string> coordsToRemove{ };
    std::vector<string> diff;
    std::sort(coords.begin(), coords.end());
    std::sort(coordsToRemove.begin(), coordsToRemove.end());
    std::set_difference(coords.begin(), coords.end(), coordsToRemove.begin(), coordsToRemove.end(),
        std::inserter(diff, diff.begin()));
    addCoordinateActuators(model, 1, 5000, true, diff);
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
        analTool.setReplaceForceSet(false);
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
    modelJR.print(outputDir + "/jr_musclelessModel.osim");
    OpenSim::Array<string> jointNames;
    modelJR.getJointSet().getNames(jointNames);
    jreact->setForcesFileName("_StaticOptimization_force.sto");
    jreact->setJointNames(jointNames);
    jreact->print("JRsetup.xml");
    jreact->setName("JointReactionIntF");
    //OpenSim::Array<std::string> onBody;
    //onBody.append("child");
    //jreact->setOnBody(onBody);
    OpenSim::AnalyzeTool analiseToolForJR(modelJR);
    setupAnalTool(analiseToolForJR, stateJR);
    analiseToolForJR.run();
    jreact->printResults("", outputDir);
}


void calculateReactionForcesWithOpensim(
    std::string modelFilename,
    const OpenSim::Storage& kinematics,
    const string& externalLoadsFilename,
    std::string outputDir) {

    OpenSim::Model model(modelFilename);
    double firstTime = kinematics.getFirstTime();
    double lastTime = kinematics.getLastTime();
    OpenSim::Array<string> coordsOS;
    model.getCoordinateSet().getNames(coordsOS);
    std::vector<string> coords = ArrayConverter::toStdVector(coordsOS);
//    std::vector<string> coordsToRemove{ "pelvis_tilt", "pelvis_list", "pelvis_rotation", "pelvis_tx", "pelvis_ty", "pelvis_tz" };
    std::vector<string> coordsToRemove{};

    std::vector<string> diff;
    std::sort(coords.begin(), coords.end());
    std::sort(coordsToRemove.begin(), coordsToRemove.end());
    std::set_difference(coords.begin(), coords.end(), coordsToRemove.begin(), coordsToRemove.end(),
        std::inserter(diff, diff.begin()));
    addCoordinateActuators(model, 1, 1000, true, diff);
    model.print(outputDir + "/static_opt_model.osim");
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
        analTool.setLowpassCutoffFrequency(6);
        analTool.setReplaceForceSet(false);
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
    modelJR.print(outputDir + "/joint_reaction_model.osim");
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
    rtb::Concurrency::Latch doneWithSubscriptions(4), doneWithExecution(4);

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
        rtb::Concurrency::Queue<QueueData<std::vector<T>>>& outputQueue,
        rtb::Concurrency::Latch& doneWithSubscriptions,
        rtb::Concurrency::Latch& doneWithExecution,
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

    rtb::Concurrency::Queue<QueueData<T>>& outputQueue_;
    rtb::Concurrency::Latch& doneWithSubscriptions_;
    rtb::Concurrency::Latch& doneWithExecution_;
    DataTable<T> dataTable_;


};

void calculateReactionForcesWithCEINMS(
    std::string osimModelFilename,
    std::string kinematicsStorageFilename,
    std::string externalLoadsXmlFilename,
    const DataTable<double>& muscleForces,
    std::string outputDir) {

    GeneralisedCoordinatesQueue generalisedCoordinateQueue;
    MultipleExternalForcesQueue multipleExternalForcesQueue;
    MuscleForcesQueue muscleForcesQueue;
    VecContactForcesQueue contactForcesQueue;
    EstimatedTorquesQueue estimatedTorquesQueue;
    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> subject;
    using MTUT = MTU<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>;
    for (auto& label : muscleForces.getLabels()) {
        subject.addMuscle(MTUT(label));
    }

    rtb::Concurrency::Latch doneWithSubscriptions(5), doneWithExecution(5);

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
        "ceinms_rct_forces",
        outputDir);

    auto producerFromDataTable([&]() {

        doneWithSubscriptions.wait();
        for (int i(0); i < muscleForces.getNRows(); ++i) {
            MuscleForcesFrame frame;
            frame.time = muscleForces.getTime(i);
            frame.data = muscleForces.getRow(i);
            muscleForcesQueue.push(frame);
        }
        muscleForcesQueue.push(EndOfData::get<MuscleForcesFrame>());
        doneWithExecution.wait();    
    });

    std::thread coordinateThr(std::ref(generalisedCoordinatesFromStorageFile));
    std::thread ifThread(std::ref(queuesToJointReactionForces));
    std::thread externalForcesThr(std::ref(externalForcesFromStorageFile));
    std::thread loggerThr(std::ref(logger));
    std::thread muscleForceThr(producerFromDataTable);

    coordinateThr.join();
    ifThread.join();
    externalForcesThr.join();
    loggerThr.join();
    muscleForceThr.join();
}


int main() {

    path dataPath(path(FileUtils::simplify(BASE_DIR)) / "Data");
    path inputPath(dataPath / "input");
    path outputPath(dataPath / "output");
    cout << "Data path " << dataPath << endl;
    string outputDir(outputPath.string());
    string osimModelFilename((inputPath / "model.osim").string());
    string kinematicsStorageFilename((inputPath / "ik.mot").string());
    string externalLoadsXmlFilename((inputPath / "ExtForcesSetup_gui.xml").string());
    
    OpenSim::Storage kinematics(kinematicsStorageFilename);
    double fc = 6;
     if (fc > 0) {
        double first = kinematics.getFirstTime();
        double last = kinematics.getLastTime();

        kinematics.pad(kinematics.getSize() / 2.);
        kinematics.lowpassIIR(fc);
        kinematics.crop(first, last);
    }


    KriggGenericCF krigg(osimModelFilename, kinematicsStorageFilename, externalLoadsXmlFilename);
    cout << "Number of muscles in Krigg model: " << krigg.getMusclesSize() << endl;
    cout << "Number of frames in Krigg model: " << krigg.getFrameNumber() << endl;
    DataTable<double> muscleForces(krigg.getFrameNumber(), krigg.getMusclesSize());
    muscleForces.setLabels(krigg.getMuscleNames());

    //calculate intersegmental forces
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

    calculateIntersegmentaForcesWithOpensim(osimModelFilename, kinematics, externalLoadsXmlFilename, outputDir);
    //calculate reaction forces
    return 0;
    calculateReactionForcesWithOpensim(osimModelFilename, kinematics, externalLoadsXmlFilename, outputDir);
    Storage muscleForcesFromStaticOpt((outputPath / "_StaticOptimization_force.sto").string());
    OpenSim::Array<string> muscleNames;
    model.getMuscles().getNames(muscleNames);
    //populate muscle forces with random values
    auto timeColumm = krigg.getTimeColumn();
    for (int c(0); c < muscleForces.getNColumns(); ++c) {
        OpenSim::Array<double> currentData;
        muscleForcesFromStaticOpt.getDataColumn(muscleNames.get(c), currentData);
        for(int r(0); r < muscleForces.getNRows(); ++r) {
            muscleForces.at(r, c) = currentData[r];
            muscleForces.setTime(timeColumm.at(r), r);
        }
    }
    auto reactionForces(krigg.getReactionForces(muscleForces));
    printDataTable(reactionForces, outputDir, "krigg_reaction1.sto");

    return 0;
}