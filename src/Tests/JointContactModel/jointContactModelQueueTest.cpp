/* -------------------------------------------------------------------------- *
* CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
* simulation. CEINMS can also be used as a plugin for OpenSim either         *
* through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
* NOTICE file for more information. CEINMS development was coordinated       *
* through Griffith University and supported by the Australian National       *
* Health and Medical Research Council (NHMRC), the US National Institutes of *
* Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
* see the PROJECTS file for more information about the funding projects.     *
*                                                                            *
* Copyright (c) 2010-2015 Griffith University and the Contributors           *
*                                                                            *
* CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
*                      E. Ceseracciu, and D.G. Lloyd                         *
*                                                                            *
* Author(s): C. Pizzolato                                                    *
*                                                                            *
* CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
* You may not use this file except in compliance with the License. You may   *
* obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
*                                                                            *
* Unless required by applicable law or agreed to in writing, software        *
* distributed under the License is distributed on an "AS IS" BASIS,          *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
* See the License for the specific language governing permissions and        *
* limitations under the License.                                             *
* -------------------------------------------------------------------------- */

#include "ceinms/QueuesToIntersegmentalForces.h"
#include "ceinms/QueuesToJointReactionForces.h"
#include "rtb/concurrency/Concurrency.h"
using namespace ceinms;
using rtb::Concurrency::Queue;
using rtb::Concurrency::Latch;
#include "ceinms/FileUtils.h"
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include <OpenSim/OpenSim.h>
#include <vector>
#include <memory>
#include "ceinms/ArrayConverter.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"
#include "ceinms/QueueToStorageFile.h"
#include <boost/filesystem.hpp>
using boost::filesystem::path;
using boost::filesystem::canonical;
using namespace OpenSim;

OpenSim::GCVSplineSet getKinematicsSplines(std::string modelFilename, Storage& kinematics, double fc) {

    OpenSim::Model model(modelFilename);
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

void calculateIntersegmentaForcesWithCEINMS(std::string osimModelFilename, std::string kinematicsStorageFilename, std::string externalLoadsXmlFilename, std::string outputDir) {

    GeneralisedCoordinatesQueue generalisedCoordinateQueue;
    MultipleExternalForcesQueue multipleExternalForcesQueue;
    VecContactForcesQueue contactForcesQueue;
    NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online> subject;
    Latch doneWithSubscriptions(4), doneWithExecution(4);

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
        osimModelFilename,
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

void  calculateIntersegmentaForcesWithOpensim(std::string modelFilename, const OpenSim::Storage& kinematics, const string& externalLoadsFilename, const vector<double>& timeColumn, std::string outputDir) {

    OpenSim::Model model(modelFilename);
    double firstTime = timeColumn.front();
    double lastTime = timeColumn.back();
    addCoordinateActuators(model, 100000, true);
    OpenSim::Model modelJR = model;
    SimTK::State state = model.initSystem();

    auto setupAnalTool([&](OpenSim::AnalyzeTool& analTool, const SimTK::State& modelState){
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
        std::string correctExtLoads((baseDir / "fixedExtLoads.xml").string());
        externalLoads.print(correctExtLoads);
        analTool.setExternalLoadsFileName(correctExtLoads);
        analTool.setStatesFromMotion(modelState, kinematics, true);
        //    analTool.updExternalLoads().setDataFileName(forcesFilename);
        analTool.setStartTime(firstTime);
        analTool.setFinalTime(lastTime);
        analTool.setPrintResultFiles(true);
        analTool.setSolveForEquilibrium(false);
        analTool.setLowpassCutoffFrequency(8);
        analTool.setReplaceForceSet(true);
    });

    //static opt
    OpenSim::StaticOptimization* sopt = new  OpenSim::StaticOptimization(&model);
    model.addAnalysis(sopt);
    OpenSim::AnalyzeTool analiseToolForSO(model);
    setupAnalTool(analiseToolForSO, state);
    //analiseToolForSO.run();
    //sopt->printResults("", outputDir);
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

int main() {

    //need to finish this validation..
    path dataPath(canonical(BASE_DIR) / "Data");
    path inputPath(dataPath / "input");
    path outputPath(dataPath / "output");

    string outputDir(outputPath.string());
    string osimModelFilename((inputPath / "model.osim").string());
    string kinematicsStorageFilename((inputPath / "ik.mot").string());
    string externalLoadsXmlFilename((inputPath / "ExtForcesSetup.xml").string());

    /*
    1. Compare output of OpenSim and CEINMS intersegmental forces calculation
    (i)   load model, disable all muscles, add external forces, load and filter kinematics
    (ii)  create CoordinateActuators for every generalised coordinate
    (iii) Osim: Run static opt + joint reaction analysis
    (iv)  CEINMS: pass model in (i) to JointContactFromOpensim and calculate intersegmental forces
    (v)   compare output from (iii) and (iv)
    */

    Storage kinematics(kinematicsStorageFilename);
    double firstTime(kinematics.getFirstTime());
    double lastTime(kinematics.getLastTime());
    OpenSim::Array<double> timeColumnOS;
    kinematics.getTimeColumn(timeColumnOS);
    vector<double> timeColumn;
    ArrayConverter::toStdVector(timeColumnOS, timeColumn);
    auto kinematicsSplines(getKinematicsSplines(osimModelFilename, kinematics, 8));
    //   calculateIntersegmentaForcesWithOpensim(osimModelFilename, kinematics, externalLoadsXmlFilename, timeColumn, outputDir);
    calculateIntersegmentaForcesWithCEINMS(osimModelFilename, kinematicsStorageFilename, externalLoadsXmlFilename, outputDir);
    /*
    InputConnectors inputConnectors;
    inputConnectors.doneWithSubscription.setCount(4);

    QueuesToIntersegmentalForces::ContactForcesQueue outputContactForcesQueue;
    using MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online > ;
    MyNMSmodel subject;

    string baseDir("C:/Users/S2849511/coding/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string osimFilename(baseDir + "model.osim");
    //
    //QueuesToIntersegmentalForces contactForcesEstimator(
    //    inputConnectors.queueGeneralisedCoordinates,
    //    inputConnectors.queueExternalForces,
    //    outputContactForcesQueue,
    //    inputConnectors.doneWithSubscription,
    //    subject,
    //    osimFilename,
    //    externalLoadsFilename,
    //    { "ankle_r" });
    //

    QueuesToJointReactionForces::MuscleForcesQueue queueMuscleForces;
    QueuesToJointReactionForces::EstimatedTorquesQueue queueTorques;

    QueuesToJointReactionForces contactForcesEstimator(inputConnectors.queueGeneralisedCoordinates,
    inputConnectors.queueExternalForces,
    queueMuscleForces,
    queueTorques,
    outputContactForcesQueue,
    inputConnectors.doneWithSubscription,
    subject,
    osimFilename,
    externalLoadsFilename,
    { "ankle_r" });

    OpenSim::Model model(osimFilename);
    model.initSystem();
    OpenSim::ExternalLoads externalLoads(model, externalLoadsFilename);
    //1 create generalised coordinates producer
    GeneralisedCoordinatesFromStorageFile generalisedCoordinateFromStorageFile(inputConnectors, osimFilename, externalLoads.getExternalLoadsModelKinematicsFileName());
    //2 create external forces produces
    ExternalForcesFromStorageFile externalForcesFromStorageFile(inputConnectors, osimFilename, externalLoadsFilename);

    //3 create and launch log
    auto logThread(std::async(std::launch::async, [&](){
    cout << "Logging\n";
    outputContactForcesQueue.subscribe();
    inputConnectors.doneWithSubscription.wait();
    double time(0);
    bool runCondition(true);
    while (runCondition) {
    auto contactForces(outputContactForcesQueue.pop());
    time = contactForces.time;
    if (time == inputConnectors.TimePlaceholderForEndOfData) runCondition = false;
    if (runCondition)
    cout << contactForces << endl;
    }
    }));

    //launch threads
    std::thread contactForcesEstimatorThread(std::ref(contactForcesEstimator));
    std::thread generalisedCoordinateThread(std::ref(generalisedCoordinateFromStorageFile));
    std::thread externalForcesThread(std::ref(externalForcesFromStorageFile));

    contactForcesEstimatorThread.join();
    generalisedCoordinateThread.join();
    externalForcesThread.join();
    logThread.wait();
    */
}