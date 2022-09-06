#include <string>
#include <iostream>
#include <filesystem>

#include "OpenSim/OpenSim.h"

#include "SimulationManager.h"
#include "ceinms/FileUtils.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/DataTable.h"

ceinms::DataTable<double> convertToDataTable(OpenSim::Storage storage) {
    // This function converts data from OpenSim Storage to CEINMS Data Table
    auto columnLabelsOS = storage.getColumnLabels();
    std::vector<std::string> columnLabels;
    ceinms::ArrayConverter::toStdVector(columnLabelsOS, columnLabels);
    OpenSim::Array<double> timeColumnOS;
    storage.getTimeColumn(timeColumnOS);
    std::vector<double> timeColumn;
    ceinms::ArrayConverter::toStdVector(timeColumnOS, timeColumn);
    ceinms::DataTable<double> dataTable;
    columnLabels.erase(columnLabels.begin());
    dataTable.setLabels(columnLabels);
    for (auto rowIndex(0); rowIndex < timeColumn.size(); rowIndex++) {
        std::vector<double> dataRow;
        for (auto columnIndex(0); columnIndex < columnLabels.size(); columnIndex++) {
            OpenSim::Array<double> dataColumnOS;
            storage.getDataColumn(columnLabelsOS.get(columnIndex + 1), dataColumnOS);
            dataRow.push_back(dataColumnOS.get(rowIndex));
        }
        dataTable.pushRow(timeColumn[rowIndex], dataRow);
    }
    return dataTable;
}

bool compareAllFiles(std::filesystem::path baseDirectory) {
    bool sameData(true);

    // Name of the output files to compare
    std::vector<string> filenames{ "Activations", "FibreLengths", "FibreVelocities", "MuscleForces", "MusclesContribution",
        "NormFibreLengths", "NormFibreVelocities", "PennationAngles", "Torques" };

    // Base directories of the output files
    std::filesystem::path previousPathBase(baseDirectory / "output - 0.10/");
    std::filesystem::path currentPathBase(baseDirectory / "output/");
    for (auto i(0); i < filenames.size(); i++) {
        // Load previous output files
        std::string previousPath = previousPathBase.string() + filenames[i] + ".sto";
        OpenSim::Storage previousData(previousPath);
        ceinms::DataTable<double> previousTable(convertToDataTable(previousData));

        // Load new output files
        std::string currentPath = currentPathBase.string() + filenames[i] + ".sto";
        OpenSim::Storage currentData(currentPath);
        ceinms::DataTable<double> currentTable(convertToDataTable(currentData));

        // Compare the two outputs and exit if they are different
        if (previousTable == currentTable && sameData)
            sameData = true;
        else
            return false;
    }

    return sameData;
}

int main() {
    std::filesystem::path setupFileName = std::filesystem::path(BASE_DIR) /= "test data/ceinmsExecutionSetupOL_ng7.xml";    // Location of the execution setup file
    ceinms::SimulationManager simulation(setupFileName.string());
    bool executionFlag = simulation.run();      // Run the CEINMS execution

    if (!executionFlag) {   // CEINMS execution completed successfully
        std::filesystem::path baseDirectory = std::filesystem::path(BASE_DIR) /= "test data";   // Base directory of the output data to compare
        if (compareAllFiles(baseDirectory))         // Compare the new output files to output from v0.10
            return 0;
    }
    return 1;
}