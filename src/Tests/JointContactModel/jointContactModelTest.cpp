#include "JointContactFromOpensim.h"
using CEINMS::JointContactFromOpensim;
using CEINMS::ExternalForce;
#include "ArrayConverter.h"

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


int main() {

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

    Storage externalForcesStorage(externalLoads.getDataFileName());
    auto externalForcesFromOsim(parseXmlForExternalForces(externalLoadsFilename));

    for (auto& f : externalForcesFromOsim) {
        f->setDataSource(externalForcesStorage);
        model.addForce(f); //model should take care of the pointers and delete them on destruction.. hopefully
    }

    JointContactFromOpensim jointContactModel(model);
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

        jointContactModel.calculateIntersegmentalForces();

  //      idSto.append(t, jointContactModel.getMobilityForces());
    
        //SimTK::Vector_<SimTK::Vec3> reactionForces, reactionTorques;
        //jointContactModel.getAllReactionForces(reactionForces, reactionTorques);
        SimTK::Vec3 reactionForce;
        jointContactModel.getReactionForceInG("talus_r", reactionForce);

        intForcesSto.append(t, reactionForce);
      //  jointContactModel.clearExternalForces();
        

    }

//    idSto.print("id_fromCeinms.sto");
    intForcesSto.print("ankle_intForces.sto");
    return 0;
}



