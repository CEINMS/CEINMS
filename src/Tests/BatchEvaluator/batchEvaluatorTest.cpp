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

#include "BatchEvaluator.h"
#include "TrialData.h"
using ceinms::TrialData;
#include "InputConnectors.h"
using ceinms::InputConnectors;
#include "OutputConnectors.h"
using ceinms::OutputConnectors;
#include "QueuesToTrialData.h"
#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "CeinmsCalibrationSetupXmlReader.h"
#include "CalibrationXmlReader.h"
#include "ExternalTorquesFromStorageFile.h"
#include "InputDataXmlReader.h"
#include "Curve.h"
#include "NMSmodel.h"
#include "Activation/ExponentialActivation.h"
#include "Activation/PiecewiseActivation.h"
//Tendon Policies
#include "Tendon/StiffTendon.h"
#include "Tendon/ElasticTendon.h"
#include "Tendon/ElasticTendon_BiSec.h"
#include "SetupDataStructure.h"
#include <thread>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <list>
using std::list;
#include <algorithm>
#include "Utilities.h"
using namespace ceinms;

void setLmtMaFilenames(const string& directory, const vector< string > dofNames, string& lmtDataFilename, vector< string >& maDataFilenames)
{
    std::string pattern{ "_Length.sto" };
    lmtDataFilename = directory + "/" + ceinms::findFile(pattern, directory);

    int currentDof = 0;
    for (auto& it : dofNames)
    {
        std::string pattern = "_MomentArm_" + it + ".sto";
        std::string maDataFilename = directory + "/" + ceinms::findFile(pattern, directory);
        maDataFilenames.push_back(maDataFilename);
    }
}

void sortMaFilenames(const std::map<string, string>& maMap, const vector< string > dofNames, vector< string >& maDataFilenames)
{
    int currentDof = 0;
    for (auto& it : dofNames)
    {
        try
        {
            maDataFilenames.push_back(maMap.at(it));
        }
        catch (std::out_of_range)
        {
            std::cerr << "Could not find moment arm file for " << it << " degree of freedom" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}


template<typename NMSmodelT>
ceinms::TrialData readTrialData(std::string inputDataFilename, NMSmodelT& mySubject, std::string trialId, std::string emgGeneratorFile)
{
    InputDataXmlReader dataLocations(inputDataFilename);
    // CEINMS::InputConnectors* inputConnectors= new CEINMS::InputConnectors();
    //CEINMS::OutputConnectors* outputConnectors = new CEINMS::OutputConnectors();

    std::unique_ptr<InputConnectors> inputConnectors(new InputConnectors);
    std::unique_ptr<OutputConnectors> outputConnectors(new OutputConnectors);

    string emgFilename(dataLocations.getExcitationsFile());
    EMGFromFile emgProducer(*inputConnectors, mySubject, emgFilename, emgGeneratorFile);

    vector< string > dofNames;
    mySubject.getDoFNames(dofNames);
    vector< string > maFilename;
    sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
    LmtMaFromStorageFile lmtMaProducer(*inputConnectors, mySubject, dataLocations.getLmtFile(), maFilename);

    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
    ExternalTorquesFromStorageFile externalTorquesProducer(*inputConnectors, mySubject, externalTorqueFilename);

    QueuesToTrialData queuesToTrialData(*inputConnectors, *outputConnectors, mySubject, trialId);


    inputConnectors->doneWithSubscription.setCount(4);
    outputConnectors->doneWithExecution.setCount(1);

    // 4. start the threads
    std::thread emgProdThread(std::ref(emgProducer));
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    std::thread queuesToTrialDataThread(std::ref(queuesToTrialData));

    emgProdThread.join();
    lmtMaProdThread.join();
    externalTorquesProdThread.join();
    queuesToTrialDataThread.join();

    if (queuesToTrialData.getTrialData().lmtData.getNRows() < 1)
    {
        std::cout << "ERROR: No data was read for trial " << trialId << std::endl;
        exit(EXIT_FAILURE);
    }
    return queuesToTrialData.getTrialData();
}

void PrintUsage()
{
    string progName = "CEINMScalibrate";
    cout << "\n\n" << progName << ":\n";// << GetVersionAndDate() << "\n\n";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}

template <typename T>
bool compareVectors(const std::vector<T>& v1, const std::vector<T>& v2) {

    return std::equal(v1.cbegin(), v1.cend(), v2.cbegin(), [](const T& a, const T& b){
        return a.equals(b); });
}


int main(int argc, char** argv) {


    string option = "";
    string setupFileName;
    if (argc < 2) {
        PrintUsage();
        return 0;
    }
    else{
        int i;
        for (i = 1; i <= (argc - 1); i++) {
            option = argv[i];

            // PRINT THE USAGE OPTIONS
            if ((option == "-help") || (option == "-h") || (option == "-Help") || (option == "-H") ||
                (option == "-usage") || (option == "-u") || (option == "-Usage") || (option == "-U")) {
                PrintUsage();
                return 0;
            }
            else if ((option == "-S") || (option == "-Setup")) {
                if (argv[i + 1] == 0){
                    cout << "No setup file specified!" << endl;
                    PrintUsage();
                    return -1;
                }
                setupFileName = argv[i + 1];
                break;

                // Print a default setup file
            }
            else if ((option == "-PrintSetup") || (option == "-PS")) {
                if (ceinms::CeinmsCalibrationSetupXmlReader::writeTemplateCeinmsCalibrationSetupFile("defaultCeinmsCalibrationSetupFile.xml"))
                {
                    std::cout << "Wrote template setup file to defaultCeinmsCalibrationSetupFile.xml" << std::endl;
                    return 0;
                }
                else
                {
                    std::cout << "An error occurred while writing template setup file to defaultCeinmsSetupFIle.xml" << std::endl;
                    return -1;
                }
            }
            else {
                cout << "Unrecognized option " << option << " on command line... Ignored" << endl;
                PrintUsage();
                return -1;
            }
        }
    }
    CeinmsCalibrationSetupXmlReader ceinmsSetup(setupFileName);

    string uncalibratedSubjectXmlFile = ceinmsSetup.getSubjectFile();
    string calibrationXmlFile = ceinmsSetup.getCalibrationFile();
    string newCalibratedSubjectXmlFile = ceinmsSetup.getOutputSubjectFile();
    string emgGeneratorFile = ceinmsSetup.getExcitationGeneratorFile();

    //1 read calibration xml
    CalibrationXmlReader calibrationXmlReader(calibrationXmlFile);

    //2 open trial files and put them in a structure
    list<string> calibrationTrialIDs;
    calibrationXmlReader.getCalibrationTrials(calibrationTrialIDs);
    vector<TrialData> trials;

    //3 loop the calibration steps
    NMSModelCfg::RunMode runMode = calibrationXmlReader.getNMSmodelRunMode();
    std::cout << "NMSModelCfg " << runMode << std::endl;
    switch (runMode) {

        case NMSModelCfg::OpenLoopExponentialActivationElasticTendonBiSecOffline: {

            using  MyNMSmodel = NMSmodel < ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline > ;
            MyNMSmodel mySubject;
            setupSubject(mySubject, uncalibratedSubjectXmlFile);
            for (list<string>::iterator trialIt = calibrationTrialIDs.begin(); trialIt != calibrationTrialIDs.end(); ++trialIt)
                trials.push_back(readTrialData(*trialIt, mySubject, *trialIt, emgGeneratorFile));
            CalibrationStep currentCalibrationStep;
            while (calibrationXmlReader.popNextCalibrationStep(currentCalibrationStep)) {
                std::cout << "CalibrationStepCfg " << currentCalibrationStep.getStepCfg() << std::endl;
                switch (currentCalibrationStep.getStepCfg()) {
                    case CalibrationCfg::MinimizeTorqueErrorParameterSetDefault: {
                        BatchEvaluator batchEvaluator(trials);
                        batchEvaluator.evaluate(mySubject);
                        auto results(batchEvaluator.getResults());

                        batchEvaluator.evaluate(mySubject);
                        auto results2(batchEvaluator.getResults());

                        cout << "Results are the same? " << compareVectors(results,results2) << std::endl;
                        std::ofstream oF1("results1.txt");
                        oF1 << results.front().activations;
                        oF1.close();

                        std::ofstream oF2("results2.txt");
                        oF2 << results2.front().activations;
                        oF2.close();



                        batchEvaluator.evaluate(mySubject);
                        auto results3(batchEvaluator.getResults());

                        cout << "Results are the same? " << compareVectors(results2, results3)<< std::endl;


                    }
                        break;
                    default:
                        std::cerr << "Calibration Step not valid\n";
                }
            } //end while


        } //end case
    break;
    }
    return 0;
}
