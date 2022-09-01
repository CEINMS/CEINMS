#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>

#include "OpenSim/OpenSim.h"

#include "ceinms/FileUtils.h"
#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/DataTable.h"

int main() {
    // Define required files to be loaded
    std::filesystem::path dataPath(std::filesystem::path(ceinms::FileUtils::simplify(BASE_DIR)) / "Data");
    std::filesystem::path inputPath(dataPath / "input");
    std::filesystem::path outputPath(dataPath / "output");
    std::cout << "Data path " << dataPath << std::endl;
    std::string outputDir(outputPath.string());
    std::string osimModelFilename((inputPath / "model.osim").string());                     // Model file
    std::string kinematicsStorageFilename((inputPath / "ik.mot").string());                 // Motion file
    std::string externalLoadsXmlFilename((inputPath / "fixedExtLoads_New.xml").string());   // External Loads files

    // Initialise joint contact model
    ceinms::JointContactFromOpensim jointContactModel(osimModelFilename, externalLoadsXmlFilename);

    // Open motion data, filter and convert to radians
    OpenSim::Model modelCopy(osimModelFilename);
    modelCopy.initSystem();
    OpenSim::Storage kinematics(kinematicsStorageFilename);
    double fc = 6;
    if (fc > 0) {
        kinematics.pad(kinematics.getSize() / 2.);
        kinematics.lowpassIIR(fc);
    }
    if (kinematics.isInDegrees()) {
        modelCopy.getSimbodyEngine().convertDegreesToRadians(kinematics);
    }

    // Create differentiable splines of the motion data to be able to get q, qd & qdd at each time point
    OpenSim::GCVSplineSet kinematicsSplines(5, &kinematics);
    //Functions must correspond to model coordinates and their order for the solver
    OpenSim::Array<std::string> coordinateNamesOS;
    modelCopy.getCoordinateSet().getNames(coordinateNamesOS);
    for (size_t i(0); i < coordinateNamesOS.getSize(); ++i) {
        if (kinematicsSplines.contains(coordinateNamesOS.get(i))) {
            kinematicsSplines.insert(i, kinematicsSplines.get(coordinateNamesOS.get(i)));
        }
        else {
            kinematicsSplines.insert(i, OpenSim::Constant(modelCopy.getCoordinateSet().get(i).getDefaultValue()));
        }
    }
    if (kinematicsSplines.getSize() > coordinateNamesOS.getSize()) {
        kinematicsSplines.setSize(coordinateNamesOS.getSize());
    }    

    // Extract the coordinate names and time column from the motion data/model
    std::vector<std::string> coordinateNames;
    ceinms::ArrayConverter::toStdVector(coordinateNamesOS, coordinateNames);
    OpenSim::Array<double> timeColumnOS;
    kinematics.getTimeColumn(timeColumnOS);
    std::vector<double> timeColumn;
    ceinms::ArrayConverter::toStdVector(timeColumnOS, timeColumn);

    // Setup output files
    std::ofstream residualMobilityForcesOutputFile;
    residualMobilityForcesOutputFile.open((outputPath / "residualMobilityForces.csv").string());
    residualMobilityForcesOutputFile << "Time, ";
    for (auto i(0); i < coordinateNames.size(); i++) {
        residualMobilityForcesOutputFile << coordinateNames[i] << ", ";
    }
    residualMobilityForcesOutputFile << ",\n";  

    ceinms::DataTable<SimTK::Vec3> jointReactionForces;
    ceinms::DataTable<SimTK::Vec3> jointReactionTorques;

    // At each time step update the joint contact model time, q, qd & qdd
    for (auto& t : timeColumn) {
        OpenSim::Array<double> currentQ, currentQd, currentQdd;
        kinematicsSplines.evaluate(currentQ, 0, t);         // Get current q at time t
        kinematicsSplines.evaluate(currentQd, 1, t);        // Get current qd at time t
        kinematicsSplines.evaluate(currentQdd, 2, t);       // Get current qdd at time t
        jointContactModel.setTime(t);                       // Set joint contact model time
        jointContactModel.setQ(currentQ);                   // Set joint contact model q
        jointContactModel.setQd(currentQd);                 // Set joint contact model qd
        jointContactModel.setQdd(currentQdd);               // Set joint contact model qdd
        jointContactModel.calculateIntersegmentalForces();  // Calculate inverse dynamics and reactions
        // Get residual mobility forces and store in output file
        SimTK::Vector residualMobilityForcesTemp;
        std::vector<double> residualMobilityForces;
        jointContactModel.getResidualMobilityForces(residualMobilityForcesTemp);
        residualMobilityForces = ceinms::ArrayConverter::fromSimTKVector(residualMobilityForcesTemp);
        residualMobilityForcesOutputFile << t << ", ";
        for (auto i(0); i < residualMobilityForces.size(); i++) {
            residualMobilityForcesOutputFile << residualMobilityForces[i] << ", ";
        }
        residualMobilityForcesOutputFile << ",\n";
        // Get reaction forces & torques and store in data table for now
        SimTK::Vector_<SimTK::Vec3> reactionForces;
        SimTK::Vector_<SimTK::Vec3> reactionTorques;
        jointContactModel.getAllReactionForces(reactionForces, reactionTorques);
        jointReactionForces.pushRow(t, ceinms::ArrayConverter::fromSimTKVector(reactionForces));
        jointReactionTorques.pushRow(t, ceinms::ArrayConverter::fromSimTKVector(reactionTorques));
    }
    residualMobilityForcesOutputFile.close();

    // Get the joint names from the joint contact model
    std::vector<std::string> jointNames;
    jointContactModel.getJointNamesFromOsimModel(jointNames);
    std::vector<std::string> jointNamesFull(jointNames);
    jointNames.erase(jointNames.begin());
    jointReactionForces.setLabels(jointNames);

    // Setup and store joint reaction forces in ouptut file
    std::ofstream jointReactionForcesOutputFile;
    jointReactionForcesOutputFile.open((outputPath / "jointReactionForces.csv").string());
    jointReactionForcesOutputFile << "Time, ";
    for (auto i(0); i < jointNames.size(); i++) {
        jointReactionForcesOutputFile << jointNames[i] << ", ";
    }
    jointReactionForcesOutputFile << ",\n";
    for (auto i(0); i<jointReactionForces.getNRows(); i++) {
        jointReactionForcesOutputFile << jointReactionForces.getTime(i) << ", ";
        for (auto m(0); m<jointNames.size(); m++) {
            double forceMagnitude = sqrt(pow(jointReactionForces.getRow(i)[m][0], 2)+pow(jointReactionForces.getRow(i)[m][1], 2)+pow(jointReactionForces.getRow(i)[m][2], 2));
            jointReactionForcesOutputFile << forceMagnitude << ", ";
        }
        jointReactionForcesOutputFile << ",\n";
    }
    jointReactionForcesOutputFile.close();

    std::ofstream jointReactionTorquesOutputFile;
    jointReactionTorquesOutputFile.open((outputPath / "jointReactionTorques.csv").string());
    jointReactionTorquesOutputFile << "Time, ";
    for (auto i(0); i < jointNamesFull.size(); i++) {
        jointReactionTorquesOutputFile << jointNamesFull[i] << ", ";
    }
    jointReactionTorquesOutputFile << ",\n";
    for (auto i(0); i<jointReactionTorques.getNRows(); i++) {
        jointReactionTorquesOutputFile << jointReactionTorques.getTime(i) << ", ";
        for (auto m(0); m<jointNamesFull.size(); m++) {
            double torqueMagnitude = sqrt(pow(jointReactionTorques.getRow(i)[m][0], 2) + pow(jointReactionTorques.getRow(i)[m][1], 2) + pow(jointReactionTorques.getRow(i)[m][2], 2));
            jointReactionTorquesOutputFile << torqueMagnitude << ", ";
        }
        jointReactionTorquesOutputFile << ",\n";
    }
    jointReactionTorquesOutputFile.close();
}