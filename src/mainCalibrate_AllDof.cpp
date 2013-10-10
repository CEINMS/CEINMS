//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <iostream>
using std::cout;
using std::endl; 
#include <string>
using std::string;
#include <stdlib.h>
#include <fstream>
using std::ofstream;
using std::ifstream;
#include <vector>
using std::vector;

  // to create the NMS model
#include "NMSmodel.h" 
#include "SetupDataStructure.h"

  // to read the emg, lmt, ma, and torque files
#include "EMGDataFromFile.h"
#include "DataFromFile.h"
#include "TorquesComputation.h"
#include "ComputationMode_Default.h"
#include "ComputationMode_Fast.h"
#include "ExponentialActivation.h"
#include "StiffTendon.h"
#include "ElasticTendon.h"
#include "Curve.h"

  // to optimize
#include "SimulatedAnnealing.h"
//#include "StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength.h"
//#include "StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_test.h"
#include "StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.h"
#include "StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.h"
//#include "SumMinObjectiveFunction.h"
#include "SumMinObjectiveFunction_singleF.h"
#include "simulatedAnnealing.hxx"
#include "subject.hxx"
//typedef ComputationMode_Default TComputationMode;
//typedef ComputationMode_Fast TComputationMode;


typedef NMSmodel<ExponentialActivation, 
                 StiffTendon, CurveMode::Offline> NMSmodel_stiff;
                 
typedef TorquesComputation<ComputationMode_Fast<NMSmodel_stiff>, 
                           NMSmodel_stiff> TorqueComputation_stiff;

                           
typedef SimulatedAnnealing<StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<NMSmodel_stiff>, 
                           SumMinObjectiveFunction_singleF<TorqueComputation_stiff>, 
                           TorqueComputation_stiff,
                           NMSmodel_stiff> Siman_stiff;
 /*                          
typedef SimulatedAnnealing<StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single, 
                           TorqueAreaDifference, 
                           ComputationMode_Fast, 
                           ExponentialActivation, 
                           StiffTendon, 
                           true> Siman_stiff_TAD;
    */                       
typedef SetupDataStructure<NMSmodel_stiff> SetupDataStructure_stiff;
/*

typedef NMSmodel<ExponentialActivation, 
                 ElasticTendon, 
                 true> NMSmodel_elastic;

typedef TorquesComputation<TComputationMode, 
                           ExponentialActivation, 
                           ElasticTendon, 
                           true> TorqueComputation_elastic;

typedef SimulatedAnnealing<StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single, 
                           SumMinObjectiveFunction_singleF, 
                           TComputationMode, 
                           ExponentialActivation, 
                           ElasticTendon, 
                           true> Siman_elastic;
                           
typedef SetupDataStructure<ExponentialActivation, ElasticTendon, true> SetupDataStructure_elastic;


typedef SimulatedAnnealing<StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single, 
                           FiberVelocitySmoothness, 
                           TComputationMode, 
                           ExponentialActivation, 
                           ElasticTendon, 
                           true> Siman_TSL_elastic;

*/


typedef NMSmodel_stiff             MyNMSmodelType;
typedef TorqueComputation_stiff    MyTorqueComputationType;
typedef Siman_stiff                MySimanType;
//typedef Siman_stiff_TAD            MySimanType;
typedef SetupDataStructure_stiff   MySetupDataStructureType;

/*
typedef NMSmodel_elastic           MyNMSmodelType;
typedef TorqueComputation_elastic  MyTorqueComputationType;
typedef SetupDataStructure_elastic MySetupDataStructureType;

typedef Siman_TSL_elastic          MySimanType;
//typedef Siman_elastic              MySimanType;
*/


int main(int argc, char** argv) {
  cout << "------------------------------------------------\n";
  cout << " NEUROMUSCOLOKELETAL MODEL: CALIBRATION \n"; 
  cout << " Calibration using simulated annealing\n";
  cout << "------------------------------------------------\n";
    
    // COMMAND LINE ARGUMENTS...
  if ( argc != 3 ) {
    cout << "Usage: calibrate InputDataDir simulatedAnnealingXMLfile\n";
    cout << "Ex; calibrate Data/NMM/Subject122bPlus simulatedAnnealing.xml\n";
    exit(EXIT_FAILURE);
  }

  string inputDataDirectory = argv[1];
  
    // The program creates three classes that will be used during optimization
    // 1. the NMS model for the subject
    // 2. the class for torque computation that, based on current values
    //    for the subject compute torques for all the DoF for all the trials
    //    for all the time steps
    // 3. Simulated annealing class
  

  
    // STEP 1. CREATE THE NMS MODEL FOR THE SUBJECT
    //         BASED ON THE "UNCALIBRATED" CONFIGURATION FROM argv[1]

  string configurationFile = inputDataDirectory + "/subject.xml";
 
  
    // just check that we have subject.xml and is formatted according to NMSmodel.xsd 
  try {
    std::auto_ptr<SubjectType> subjectPointer (subject ( configurationFile.c_str() ) );
  }  
  catch (const xml_schema::exception& e) {
    cout << e << endl;
    exit(EXIT_FAILURE);
  }
  
    // create the NMS model of the subject based on a argv[1] XML file 
  cout << "Create data structure...\n";
  MyNMSmodelType mySubject;
  
    // create the class that from the xml file setup NMSmodel members
  MySetupDataStructureType setupData(configurationFile); 
    // setup Curves
  cout << "Create curves...\n";
  setupData.createCurves();
    // setup Muscles
  cout << "Create muscles...\n";
  setupData.createMuscles(mySubject);
    // setup DoF
  cout << "Create DoF...\n";
  setupData.createDoFs(mySubject);
  
  cout << "OUR STARTING SUBJECT IS:\n";
  cout << mySubject;

    // STEP 2. CREATE THE CLASS THAT IS READING THE INPUT FILES WITH THE DATA 
    // THAT WILL BE USED FOR THE ANNEALING

    // read trials name
  
  string trialsFileName = inputDataDirectory + "/trials.txt";
  
  ifstream trialsFile(trialsFileName.c_str());
  
  if (!trialsFile.is_open()) {
    cout << "ERROR: " << trialsFileName << " could not be open\n";
    exit(EXIT_FAILURE);
  }
    
  string trash;
  trialsFile >> trash;
  int noTrials;
  trialsFile >> noTrials;
  
  vector<string> idTrials;
  
  for (int it = 0; it < noTrials; ++it) { 
    string nextTrialName;
    trialsFile >> nextTrialName;
    idTrials.push_back(nextTrialName);
  }
  
  
            
  
  vector<string> dofsToCalibrate;

    // STEP 3. CREATE THE SIMULATED ANNEALING FOR THE NMS MODEL
    //         DATA FOR THE OPTIMIZATION ARE IN argv[2]

    // this is only to check that the argv[2] argument is a proper xml file
  try {
    std::auto_ptr<SimulatedAnnealingType> annealingPointer (simulatedAnnealing(argv[2]));
  } 
  catch(const xml_schema::exception& e) {
    cout << e << endl;  
    exit(EXIT_FAILURE);
  }

    // Simulated annealing has two policies:
    // the first one represents the parameters we want to optimize
    // the second one the objective function we want to use
  string confFile = argv[2];

  // STEP 4. OPTIMIZE :)

    dofsToCalibrate.push_back("R_knee_fe");
    dofsToCalibrate.push_back("R_hip_fe");
    dofsToCalibrate.push_back("R_hip_aa");
  //  dofsToCalibrate.push_back("R_hip_rot");
    dofsToCalibrate.push_back("R_ankle_fe");
  //  dofsToCalibrate.push_back("R_ankle_sa");
    MyTorqueComputationType torquesComputation(mySubject, 
                                                                inputDataDirectory, 
                                                                idTrials, 
                                                                dofsToCalibrate);
    MySimanType annealing(mySubject, dofsToCalibrate, confFile, torquesComputation);
    annealing.optimize();
    
  // STEP 5. WRITE THE DATA OF THE SUBJECT TO AN XML FILE
  //         READ CURRENT DATA FROM THE SUBJECT AND UPDATE THE XML
  cout << "\n NOW WRITE THE CALIBRATED xml: \n";
  string XMLfile = inputDataDirectory + "/subjectCalibrated.xml";
  setupData.writeXMLCalibratedFile(mySubject, XMLfile);

  
  
  
  exit(EXIT_SUCCESS);

}
