#include "JointContactFromOpensim.h"
using CEINMS::JointContactFromOpensim;
using CEINMS::ExternalForce;
#include "ArrayConverter.h"

#include <OpenSim/OpenSim.h>
using OpenSim::Storage;
#include <OpenSim/Tools/InverseDynamicsTool.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

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


/*void checkKinematicsParsing(OpenSim::GCVSplineSet& kinematicsSplines, vector<double> timecolumn) {
    Storage qStorage, qdStorage, qddStorage;
    for (auto t : timecolumn) {
        OpenSim::StateVector sv;
        kinematicsSplines.evaluate()
        sv.setStates(t,)
    }
}
*/
int main() {

    string baseDir("C:/Users/Claudio/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string modelFilename(baseDir + "model.osim");

    OpenSim::Model model(modelFilename);
    JointContactFromOpensim jointContactModel(model);

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

    //1 check kinematics parsing (position, velocity, acceleration)
 //   checkKinematicsParsing(kinematicsSplines, timeColumn);

    //2 check forces parsing
    //3 check id results

    //setup external forces
    Storage externalForcesStorage(externalLoads.getDataFileName());
    auto externalForcesFromOsim(parseXmlForExternalForces(externalLoadsFilename));
    for (auto& f : externalForcesFromOsim) {
        f->setDataSource(externalForcesStorage);
        model.addComponent(f); //model should take care of the pointers and delete them on destruction.. hopefully
    }


    Storage idSto, qSto, qdSto, qddSto;
    OpenSim::Array<std::string> labels;
    model.getCoordinateSet().getNames(labels);
    labels.insert(0, "time");
    idSto.setColumnLabels(labels);
    qSto.setColumnLabels(labels);
    qdSto.setColumnLabels(labels);
    qddSto.setColumnLabels(labels);

    const OpenSim::BodySet& bodies(model.getBodySet());
    for (auto t : timeColumn) {


        for (auto& f : externalForcesFromOsim) {
            ExternalForce force;
            force.setTargetBody(bodies.getIndex(f->getAppliedToBodyName()));
            force.setForceVector(f->getForceAtTime(t), bodies.getIndex(f->getForceExpressedInBodyName()));
            force.setForceApplicationPoint(f->getPointAtTime(t), bodies.getIndex(f->getPointExpressedInBodyName()));
            force.setTorque(f->getTorqueAtTime(t));
      //      force.print(std::cout);
            jointContactModel.addExternalForce(force);
        }
        jointContactModel.setTime(t);

        OpenSim::Array<double> q;
        kinematicsSplines.evaluate(q, 0, t);
        jointContactModel.setQ(q);
        qSto.append(t, q);

        OpenSim::Array<double> qd;
        kinematicsSplines.evaluate(qd, 1, t);
        jointContactModel.setQd(qd);
        qdSto.append(t, qd);

        OpenSim::Array<double> qdd;
        kinematicsSplines.evaluate(qdd, 2, t);
        jointContactModel.setQdd(qdd);
        qddSto.append(t, qdd);

        jointContactModel.calculateIntersegmentalForces();
//        JointContactFromOpensim::VecSPVec reactionForces;
//        jointContactModel.getAllReactionForces(reactionForces);

//        std::cout << t << '\t' <<jointContactModel.getMobilityForces() << std::endl;
        idSto.append(t, jointContactModel.getMobilityForces());

        jointContactModel.clearExternalForces();
        

    }

    idSto.print("id_fromCeinms_wG.sto");
    qSto.print("q.sto");
    qdSto.print("qd.sto");
    qddSto.print("qdd.sto");


    Storage kin(kinematicsFilename);
    model.getSimbodyEngine().convertDegreesToRadians(kin);
    kin.print("kin_rad.sto");

/*    OpenSim::Array<string> muscleNames;
    model.getMuscles().getNames(muscleNames);
    OpenSim::InverseDynamicsTool idTool;
    idTool.setModel(model);
    idTool.setCoordinatesFileName(kinematicsFilename);
    idTool.setExternalLoadsFileName(externalLoadsFilename);
    idTool.setResultsDir("IdSolution");
    idTool.setExcludedForces(muscleNames);
    idTool.run(); */
    return 0;
}



