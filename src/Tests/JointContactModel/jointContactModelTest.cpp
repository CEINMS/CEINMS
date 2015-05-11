#include "JointContactFromOpensim.h"
#include "ArrayConverter.h"
#include "ContactModelXmlReader.h"
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

OpenSim::GCVSplineSet getKinematicsSplines(OpenSim::Model& model, Storage& kinematics, double fc) {

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


std::vector<OpenSim::ExternalForce*> parseXmlForExternalForces(const string& xmlFilename) {
    OpenSim::XMLDocument document(xmlFilename);
    SimTK::Xml::Element root = document.getRootDataElement();
    SimTK::Xml::element_iterator extForceElementIter = root.element_begin("objects")->element_begin("ExternalForce");
    std::vector<OpenSim::ExternalForce*> extForces;
    for (extForceElementIter; extForceElementIter != root.element_begin("objects")->element_end(); ++extForceElementIter) {
        OpenSim::ExternalForce* ef = new OpenSim::ExternalForce(*extForceElementIter);
        extForces.push_back(ef);
    }
    return extForces;
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

void calculateIntersegmentaForcesWithCEINMS(const OpenSim::Model& aModel, const OpenSim::GCVSplineSet& kinematicsSplines, const vector<double>& timeColumn) {
    
  /*  OpenSim::Model model(aModel);

    JointContactFromOpensim jointContactModel(model.getInputFileName(), );
    auto modelSO(model);
    string jointName("ankle_r");
    //  OpenSim::StaticOptimization staticOpt;
    //  staticOpt.
    //you should create a derived class of OpenSim::Force, which has a reference to a queue (for real time) or an internal storage..
    //everytime a force is requested it pops it

    OpenSim::Array<std::string> labels;
    model.getCoordinateSet().getNames(labels);
    labels.insert(0, "time");
    Storage idSto, intForcesSto;
    idSto.setColumnLabels(labels);
    const OpenSim::BodySet& bodies(model.getBodySet());


    for (auto t : timeColumn) {

        jointContactModel.setTime(t);

        OpenSim::Array<double> q;
        kinematicsSplines.evaluate(q, 0, t);
        jointContactModel.setQ(q);

        OpenSim::Array<double> qd;
        kinematicsSplines.evaluate(qd, 1, t);
        jointContactModel.setQd(qd);

        OpenSim::Array<double> qdd;
        kinematicsSplines.evaluate(qdd, 2, t);
        jointContactModel.setQdd(qdd);

        jointContactModel.calculateReactionForces(true);

        SimTK::Vec3 reactionForce;
        jointContactModel.getReactionForceInG(jointName, reactionForce);
        idSto.append(t, jointContactModel.getMobilityForces());
        intForcesSto.append(t, reactionForce);

    }

    string parentBodyName(model.getJointSet().get(jointName).getParentName());
    string childBodyName(model.getJointSet().get(jointName).getBody().getName());
    string columnName(jointName + "_on_" + childBodyName + "_in_" + model.getGroundBody().getName());
    OpenSim::Array<std::string> intForcesLabels;
    intForcesLabels.append("time");
    intForcesLabels.append(columnName + "_fx");
    intForcesLabels.append(columnName + "_fy");
    intForcesLabels.append(columnName + "_fz");
    intForcesSto.setColumnLabels(intForcesLabels);
    intForcesSto.print("ankle_intForces.sto");
    idSto.print("id_fromCeinms.sto");
    */
}

//copy the model
void  calculateIntersegmentaForcesWithOpensim(OpenSim::Model model, const OpenSim::Storage& kinematics, const string& externalLoadsFilename, const vector<double>& timeColumn) {
    
    double firstTime = timeColumn.front();
    double lastTime = timeColumn.back();
    addCoordinateActuators(model, 100000, true);
    OpenSim::Model modelJR = model;
    SimTK::State state = model.initSystem();

    auto setupAnalTool([&](OpenSim::AnalyzeTool& analTool, const SimTK::State& modelState){
  //      analTool.setCoordinatesFileName(kinematicsFilename);
        analTool.setStatesFromMotion(modelState, kinematics, true);
        analTool.setExternalLoadsFileName(externalLoadsFilename);
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
    analiseToolForSO.run();
    model.removeAnalysis(sopt, false);
    
    //joint reaction
    SimTK::State stateJR = modelJR.initSystem();
    OpenSim::JointReaction* jreact = new OpenSim::JointReaction(&modelJR);
    modelJR.addAnalysis(jreact);
    OpenSim::Array<string> jointNames;
    jreact->setForcesFileName("_StaticOptimization_force.sto");
    jointNames.append("ankle_r");
    jreact->setJointNames(jointNames);
    jreact->print("JRsetup.xml");
    jreact->setName("JointReaction");
    OpenSim::AnalyzeTool analiseToolForJR(modelJR);
    setupAnalTool(analiseToolForJR, stateJR);
    analiseToolForJR.run();
   
}


void  calculateContactForcesWithOpensim(OpenSim::Model model, const OpenSim::Storage& kinematics, const string& externalLoadsFilename, const vector<double>& timeColumn) {

    for (unsigned i(0); i < model.getMuscles().getSize(); ++i) {
        auto f = model.getMuscles().get(i).getMaxIsometricForce();
        model.getMuscles().get(i).setMaxIsometricForce(f*3.0);
    }

    double firstTime = timeColumn.front();
    double lastTime = timeColumn.back();
    addCoordinateActuators(model, 10000, false);
    OpenSim::Model modelJR = model;
    SimTK::State state = model.initSystem();

    auto setupAnalTool([&](OpenSim::AnalyzeTool& analTool, const SimTK::State& modelState){
        //      analTool.setCoordinatesFileName(kinematicsFilename);
        analTool.setStatesFromMotion(modelState, kinematics, true);
        analTool.setExternalLoadsFileName(externalLoadsFilename);
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
    analiseToolForSO.run();
    model.removeAnalysis(sopt, false);

    //joint reaction
    SimTK::State stateJR = modelJR.initSystem();
    OpenSim::JointReaction* jreact = new OpenSim::JointReaction(&modelJR);
    modelJR.addAnalysis(jreact);
    OpenSim::Array<string> jointNames;
    jreact->setForcesFileName("_StaticOptimization_force.sto");
    jointNames.append("ankle_r");
    jreact->setJointNames(jointNames);
    jreact->print("JRsetup.xml");
    jreact->setName("JointReaction_CF");
    OpenSim::AnalyzeTool analiseToolForJR(modelJR);
    setupAnalTool(analiseToolForJR, stateJR);
    analiseToolForJR.run();

}

int main() {

    string xmlFilename("C:/Users/S2849511/Dropbox/Ricerca/Data/ContactModelTest/contactModel.xml");
    ContactModelXmlReader xmlReader(xmlFilename);
    ContactModelFromOpenSimSetup setup = xmlReader.getContactModelSetupOsim();
    cout << setup.osimModel << endl;
    cout << setup.motion << endl;
    cout << setup.externalLoads << endl;
    for (auto i : setup.joints)
        cout << i << endl;


    /*
    1. Compare output of OpenSim and CEINMS intersegmental forces calculation
        (i)   load model, disable all muscles, add external forces, load and filter kinematics
        (ii)  create CoordinateActuators for every generalised coordinate
        (iii) Osim: Run static opt + joint reaction analysis
        (iv)  CEINMS: pass model in (i) to JointContactFromOpensim and calculate intersegmental forces
        (v)   compare output from (iii) and (iv)
    2. Compare output of OpenSim and CEINMS reaction forces calculation, 
       same muscle forces and residual calculated in OpenSim should give the same results in CEINMS 
        (i)   load model, add external forces, load and filter kinematics
        (ii)  Osim: Run static opt + joint reaction analysis
        (iii) CEINMS: pass model in (i) to JointContactFromOpensim
        (iv)  get muscle forces and residual from (ii) and pass them to JointContactFromOpensim
        (v)   calculate joint reaction forces with JointContactFromOpensim
        (vi)  compare (ii) and (v)
    */

    /*

    string baseDir("C:/Users/S2849511/coding/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string modelFilename(baseDir + "model.osim");

    OpenSim::Model model(modelFilename);
    OpenSim::ExternalLoads externalLoads(model, externalLoadsFilename);

    //setup kinematics
    string kinematicsFilename(externalLoads.getExternalLoadsModelKinematicsFileName());
    double fc(externalLoads.getLowpassCutoffFrequencyForLoadKinematics());
    Storage kinematics(kinematicsFilename);
    double firstTime(kinematics.getFirstTime());
    double lastTime(kinematics.getLastTime());
    OpenSim::Array<double> timeColumnOS;
    kinematics.getTimeColumn(timeColumnOS);
    vector<double> timeColumn;
    ArrayConverter::toStdVector(timeColumnOS, timeColumn);
    auto kinematicsSplines(getKinematicsSplines(model, kinematics, fc));

    //setup external forces
    Storage externalForcesStorage(externalLoads.getDataFileName());
    auto externalForcesFromOsim(parseXmlForExternalForces(externalLoadsFilename));
    for (auto& f : externalForcesFromOsim) {
        f->setDataSource(externalForcesStorage);
        model.addForce(f); //model should take care of the pointers and delete them on destruction.. hopefully
    }

    // 1. Compare output of OpenSim and CEINMS intersegmental forces calculation
    // successfully tested
    if (false) {
        calculateIntersegmentaForcesWithCEINMS(model, kinematicsSplines, timeColumn);
        cout << "----------\nOsim static opt\n------------\n";
        calculateIntersegmentaForcesWithOpensim(model, kinematics, externalLoadsFilename, timeColumn);
    }
    //  2. Compare output of OpenSim and CEINMS reaction forces calculation, 
    //     same muscle forces and residual calculated in OpenSim should give the same results in CEINMS
    calculateContactForcesWithOpensim(model, kinematics, externalLoadsFilename, timeColumn);
    */


    return 0;
}



