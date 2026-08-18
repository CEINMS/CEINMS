#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include <stdexcept>
#include <sstream>

#include "OpenSim/OpenSim.h"

#include "ceinms/FileUtils.h"
#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/DataTable.h"

OpenSim::Storage initialiseKinematicData(OpenSim::Model modelCopy, std::string kinematicsFilename) {
    // Open motion data, filter and convert to radians
    
    OpenSim::Storage kinematics(kinematicsFilename);
    double fc = 6;
    if (fc > 0) {
        kinematics.pad(kinematics.getSize() / 2.);
        kinematics.lowpassIIR(fc);
    }
    if (kinematics.isInDegrees()) {
        modelCopy.getSimbodyEngine().convertDegreesToRadians(kinematics);
    }
    return kinematics;
}

OpenSim::GCVSplineSet initialiseKinematicSpline(OpenSim::Storage kinematics, OpenSim::Model model) {
    OpenSim::GCVSplineSet kinematicsSplines(5, &kinematics);
    OpenSim::Array<std::string> coordinateNames;
    model.getCoordinateSet().getNames(coordinateNames);

    for (size_t i(0); i < coordinateNames.getSize(); ++i) {
        if (kinematicsSplines.contains(coordinateNames.get(i))) {
            kinematicsSplines.insert(i, kinematicsSplines.get(coordinateNames.get(i)));
        }
        else {
            kinematicsSplines.insert(i, OpenSim::Constant(model.getCoordinateSet().get(i).getDefaultValue()));
        }
    }
    if (kinematicsSplines.getSize() > coordinateNames.getSize()) {
        kinematicsSplines.setSize(coordinateNames.getSize());
    }

    return kinematicsSplines;
}

void storeDataInFile(std::filesystem::path filepath, ceinms::DataTable<SimTK::Vec3> dataTable) {
    std::ofstream outputFile;
    std::vector<std::string> jointNames(dataTable.getLabels());
    outputFile.open(filepath.string());
    outputFile << "Time";
    for (auto i(0); i < jointNames.size(); i++) {
        outputFile << "," << jointNames[i] ;
    }
    outputFile << "\n";
    for (auto i(0); i < dataTable.getNRows(); i++) {
        outputFile << dataTable.getTime(i);
        for (auto m(0); m < jointNames.size(); m++) {
            double forceMagnitude = sqrt(pow(dataTable.getRow(i)[m][0], 2) + pow(dataTable.getRow(i)[m][1], 2) + pow(dataTable.getRow(i)[m][2], 2));
            outputFile << "," << forceMagnitude ;
        }
        outputFile << "\n";
    }
    outputFile.close();
}

void storeDataInFile(std::filesystem::path filepath, ceinms::DataTable<double> dataTable) {
    std::ofstream outputFile;
    std::vector<std::string> labels(dataTable.getLabels());
    outputFile.open(filepath.string());
    outputFile << "Time";
    for (auto i(0); i < labels.size(); i++) {
        outputFile << "," << labels[i];
    }
    outputFile << "\n";
    for (auto i(0); i < dataTable.getNRows(); i++) {
        outputFile << dataTable.getTime(i);
        for (auto m(0); m < labels.size(); m++) {
            outputFile << "," << dataTable.getRow(i)[m];
        }
        outputFile << "\n";
    }
    outputFile.close();
}

ceinms::DataTable<double> loadDataFromFile(std::filesystem::path filepath) {
    ceinms::DataTable<double> forceDataTable;

    std::ifstream dataFile(filepath);   // Create an input filestream   
    if (!dataFile.is_open()) throw std::runtime_error("Could not open previous data file");  // Make sure the file is open
    std::string line, columnName;
    double val;
    std::vector<std::string> columnNames;
    if (dataFile.good()) {
        std::getline(dataFile, line);   // Extract the first line in the file
        std::stringstream ss(line);     // Create a stringstream from the current line
        while (std::getline(ss, columnName, ',')) {     // Extract each column name
            columnNames.push_back(columnName);      // Store column names in vector
        }
    }
    columnNames.erase(columnNames.begin());
    forceDataTable.setLabels(columnNames);
    std::vector<double> timeData;
    std::vector<std::vector<double>> forceData;
    forceData.resize(columnNames.size());
    while (std::getline(dataFile, line)) {  // Read data line by line
        std::stringstream ss(line); //Create a stringstream from the current line
        std::string token;
        int columnIndex(0);
        while (std::getline(ss, token, ',')) {
            val = std::stod(token); // std::stod (unlike operator>>) also parses "inf"/"nan"
            if (columnIndex == 0) {
                timeData.push_back(val);
            }
            else {
                forceData[columnIndex - 1].push_back(val);
            }
            columnIndex++;
        }
    }
    dataFile.close();

    for (auto i(0); i < timeData.size(); i++) {
        std::vector<double> newDataRow;
        for (auto forceIndex(0); forceIndex < forceData.size(); forceIndex++) {
                newDataRow.push_back(forceData[forceIndex][i]);
        }
        forceDataTable.pushRow(timeData[i], newDataRow);
    }

    return forceDataTable;
}

bool compareData(ceinms::DataTable<double> data1, ceinms::DataTable<double> data2, double accuracy) {
    bool sameData(true);
    if (data1.getNRows() == data2.getNRows() && data1.getNColumns() == data2.getNColumns()) {
        for (auto i(0); i < data1.getNColumns(); i++) {
            std::vector<double> data1Vector(data1.getColumn(i));
            std::vector<double> data2Vector(data2.getColumn(i));
            for (auto r(0); r < data1.getNRows(); r++) {
                double data1Value(data1Vector[r]);
                double data2Value(data2Vector[r]);
                double testValue = abs(data1Value - data2Value);
                if (testValue > accuracy) {
                    std::cout << "Reaction force different!";
                    std::cout << "(" << data1.getLabel(i) << " @ Time: " << data1.getTime(r) << ")" << std::endl;
                    sameData = false;
                }
            }
        }
        if (sameData) {
            std::cout << "All reaction forces the same!" << std::endl;
        }
    }
    else {
        std::cout << "Different vector sizes!" << std::endl;
        sameData = false;
    }
    return sameData;
}

int main() {
    // Define required files to be loaded
    std::filesystem::path dataPath(std::filesystem::path(ceinms::FileUtils::simplify(BASE_DIR)) / "Data");
    std::filesystem::path inputPath(dataPath / "input");
    std::filesystem::path outputPath(dataPath / "output");
    std::cout << "Data path " << dataPath << std::endl;
    std::string outputDir(outputPath.string());
    std::string osimModelFilename((inputPath / "model.osim").string());                     // Model file
    std::string kinematicsStorageFilename((inputPath / "ik.mot").string());                 // Motion file
    std::string externalLoadsXmlFilename((inputPath / "ExternalLoadsSetup.xml").string());   // External Loads files

    // Initialise joint contact model
    ceinms::JointContactFromOpensim jointContactModel(osimModelFilename, externalLoadsXmlFilename);

    OpenSim::Model modelCopy(osimModelFilename);
    modelCopy.initSystem();
    OpenSim::Storage kinematics(initialiseKinematicData(modelCopy, kinematicsStorageFilename));

    // Create differentiable splines of the motion data to be able to get q, qd & qdd at each time point
    OpenSim::GCVSplineSet kinematicsSplines(initialiseKinematicSpline(kinematics, modelCopy));
    
    // Extract the coordinate names and time column from the motion data/model
    OpenSim::Array<std::string> coordinateNamesOS;
    modelCopy.getCoordinateSet().getNames(coordinateNamesOS);
    std::vector<std::string> coordinateNames;
    ceinms::ArrayConverter::toStdVector(coordinateNamesOS, coordinateNames);
    OpenSim::Array<double> timeColumnOS;
    kinematics.getTimeColumn(timeColumnOS);
    std::vector<double> timeColumn;
    ceinms::ArrayConverter::toStdVector(timeColumnOS, timeColumn);

    ceinms::DataTable<SimTK::Vec3> jointReactionForces;
    ceinms::DataTable<SimTK::Vec3> jointReactionTorques;
    ceinms::DataTable<double> residualMobilityForces;
    
    // At each time step update the joint contact model time, q, qd & qdd then calculate inverse dynamics and reactions
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
        // Get reaction forces & torques and store in data table for now
        SimTK::Vector_<SimTK::Vec3> reactionForces;
        SimTK::Vector_<SimTK::Vec3> reactionTorques;
        jointContactModel.getAllReactionForces(reactionForces, reactionTorques);
        jointReactionForces.pushRow(t, ceinms::ArrayConverter::fromSimTKVector(reactionForces));
        jointReactionTorques.pushRow(t, ceinms::ArrayConverter::fromSimTKVector(reactionTorques));
        // Get residual mobility forces and store in output file
        SimTK::Vector residualMobilityForcesTemp;
        jointContactModel.getResidualMobilityForces(residualMobilityForcesTemp);
        residualMobilityForces.pushRow(t, ceinms::ArrayConverter::fromSimTKVector(residualMobilityForcesTemp));
    }

    // Set data table labels
    std::vector<std::string> jointNames;
    jointContactModel.getJointNamesFromOsimModel(jointNames);
    jointReactionTorques.setLabels(jointNames);
    jointReactionForces.setLabels(jointNames);
    residualMobilityForces.setLabels(coordinateNames);
    
    // Store the reaction forces and torques in a csv file
    storeDataInFile((outputPath / "jointReactionForces.csv"), jointReactionForces);
    storeDataInFile((outputPath / "jointReactionTorques.csv"), jointReactionTorques);
    storeDataInFile((outputPath / "residualMobilityForces.csv"), residualMobilityForces);

    // Compare the computed reaction force data to previous data
    ceinms::DataTable<double> previousData = loadDataFromFile(inputPath / "jointReactionForces.csv");
    ceinms::DataTable<double> currentData = loadDataFromFile(outputPath / "jointReactionForces.csv");
    if (compareData(previousData, currentData, 0.01))    // 0.01 is how close together they have to be to pass
        return 0;
    return 1;
}