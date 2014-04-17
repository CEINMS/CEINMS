# SET(SimAnnealObjectiveFunctions_SOURCES
#   ObjectiveFunctions/SumMinObjectiveFunction_singleF.cpp
# 
# )
# SET(SimAnnealParameters_SOURCES
 # SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength.cpp
 # SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.cpp
 # SimAnnealParameters/StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.cpp
 # SimAnnealParameters/TendonSlackLength.cpp
#)


#SET(AmoebaObjectiveFunctions_SOURCES
#  ObjectiveFunctions/ObjFun_Amoeba.cpp
#)

#SET(AmoebaParameters_SOURCES
#  AmoebaParameters/Param_Amoeba.cpp
#)

# SET(TorqueComputationModes_SOURCES
#  TorqueComputationModes/ComputationMode_Default.cpp
#  TorqueComputationModes/ComputationMode_Fast.cpp
# )

#SET(FileIO_SOURCES
#   ${CMAKE_SOURCE_DIR}/lib/FileIO/DataFromFile.cpp
#   ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGDataFromFile.cpp
#   ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator/EMGgeneratorFrom10To13.cpp
#   ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator/EMGgeneratorFrom16To34.cpp
#   ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator/EMGgeneratorFrom6To24.cpp
# )


include_directories(
  ${xmlInput_INCLUDE_DIRS}
  ${NMSmodel_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${FileIO_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ceinms_common_INCLUDE_DIRS}
)

include_directories(${Boost_INCLUDE_DIRS})

set(Calibration_HEADERS TrialData.h
                        TorquesComputation.h
                        SimulatedAnnealing.h
                        InputDataInterpreter.h
                        ObjectiveFunctions/ObjectiveFunction.h
                        ObjectiveFunctions/SumMinObjectiveFunction_singleF.h
                        SimAnnealParameters/ParametersFromXml.h
                        SimAnnealParameters/ParametersToOptimize.h
                        SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.h
                        SimAnnealParameters/StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.h
                        TorqueComputationModes/ComputationMode_Default.h
                        TorqueComputationModes/ComputationMode_Fast.h
                        )
set(Calibration_SOURCES TrialData.cpp
                        TorquesComputation.cpp
                        SimulatedAnnealing.cpp
                        InputDataInterpreter.cpp
                        ObjectiveFunctions/SumMinObjectiveFunction_singleF.cpp
                        SimAnnealParameters/ParametersFromXml.cpp
                        SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.cpp
                        SimAnnealParameters/StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.cpp
                        TorqueComputationModes/ComputationMode_Default.cpp
                        TorqueComputationModes/ComputationMode_Fast.cpp
                        )
set_source_files_properties(TrialData.cpp
                            TorquesComputation.cpp
                            SimulatedAnnealing.cpp
                            ObjectiveFunctions/SumMinObjectiveFunction_singleF.cpp
                            SimAnnealParameters/ParametersFromXml.cpp
                            SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.cpp
                            SimAnnealParameters/StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.cpp
                            TorqueComputationModes/ComputationMode_Default.cpp
                            TorqueComputationModes/ComputationMode_Fast.cpp
                            PROPERTIES HEADER_FILE_ONLY TRUE)
if(OPENSIM_FOUND)
  include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS})
  list(APPEND Calibration_HEADERS InputDataInterpreterOsim.h )
  list(APPEND Calibration_SOURCES InputDataInterpreterOsim.cpp )
endif()

source_group("Header Files" FILES ${Calibration_HEADERS})
source_group("Source Files" FILES ${Calibration_SOURCES})

add_library(Calibration STATIC ${Calibration_SOURCES} ${Calibration_HEADERS})
target_link_libraries(Calibration xmlInputCalibration FileIO)

if(OPENSIM_FOUND)
  target_link_libraries(Calibration ${OPENSIM_LIBRARIES}) #should find out which ones are actually needed
endif()

#  SimulatedAnnealing.cpp 
 # Amoeba.cpp
 # TorquesComputation.cpp
#  TrialData.cpp
#  ${TorqueComputationModes_SOURCES}
#  ${CMAKE_SOURCE_DIR}/lib/NMSmodel/MuscleParameters.cpp
#  ${FileIO_SOURCES}
#  ${SimAnnealObjectiveFunctions_SOURCES}
#  ${SimAnnealParameters_SOURCES}
 # ${AmoebaObjectiveFunctions_SOURCES}
 # ${AmoebaParameters_SOURCES}
#)

set(Calibration_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ObjectiveFunctions ${CMAKE_CURRENT_SOURCE_DIR}/SimAnnealParameters  ${CMAKE_CURRENT_SOURCE_DIR}/TorqueComputationModes CACHE STRING "Include directories for Calibration library")

