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

if(OPENSIM_FOUND)
  include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS})
  set(Calibration_SOURCE InputDataInterpreter.cpp InputDataInterpreterOsim.cpp)
else()
  set(Calibration_SOURCE InputDataInterpreter.cpp)
endif()
add_library(Calibration STATIC ${Calibration_SOURCE})
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

