#include "JointContactFromOpensim.h"
using CEINMS::JointContactFromOpensim;
using CEINMS::ExternalForce;
#include "ArrayConverter.h"

#include <OpenSim/OpenSim.h>
using OpenSim::Storage;

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


OpenSim::ModelComponentSet<OpenSim::ExternalForce> parseXmlForExternalForces(const string& xmlFilename) {
    OpenSim::XMLDocument document(xmlFilename);
    SimTK::Xml::Element root = document.getRootDataElement();
    SimTK::Xml::element_iterator extForceElementIter = root.element_begin("objects")->element_begin("ExternalForce");
    OpenSim::ModelComponentSet<OpenSim::ExternalForce> extForces;
    for (extForceElementIter; extForceElementIter != root.element_begin("objects")->element_end(); ++extForceElementIter) {
        OpenSim::ExternalForce ef(*extForceElementIter);
        extForces.cloneAndAppend(ef);
    }
    return extForces;
}

int main() {

    string baseDir("C:/Users/Claudio/versioning/ceinms/src/Tests/JointContactModel/Data/");
    string externalLoadsFilename(baseDir + "ExtForcesSetup.xml");
    string modelFilename(baseDir + "model.osim");

    OpenSim::Model model(modelFilename);
  //  JointContactFromOpensim jointContactModel(model);

    OpenSim::ExternalLoads externalLoads(model, externalLoadsFilename);

    //setup kinematics
    string kinematicsFilename(externalLoads.getExternalLoadsModelKinematicsFileName());
    double fc(externalLoads.getLowpassCutoffFrequencyForLoadKinematics());
    Storage kinematics(kinematicsFilename);
 //   auto kinematicsSplines(getKinematicsSplines(model, kinematics, fc));
    auto firstTime(kinematics.getFirstTime());
    auto lastTime(kinematics.getLastTime());

    OpenSim::Array<double> timeColumnOS;
    kinematics.getTimeColumn(timeColumnOS);
    vector<double> timeColumn;
    ArrayConverter::toStdVector(timeColumnOS, timeColumn);


        
        //setup external forces
    Storage externalForcesStorage(externalLoads.getDataFileName());
    auto externalForcesSetup(parseXmlForExternalForces(externalLoadsFilename));
    // GET DOCUMENT ELEMENT
    externalForcesSetup[1].setDataSource(externalForcesStorage);
    OpenSim::ExternalForce* p1 = new OpenSim::ExternalForce(externalForcesSetup[1]);
    model.addComponent(p1);
    for (auto t : timeColumn) {
        std::cout << "Time: " << t << "\tVal: " << p1->getPointAtTime(t) << std::endl;
    }
  //  OpenSim::ExternalForce extForce(aNode);
    delete p1;
    return 0;
}



