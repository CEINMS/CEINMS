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


INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/lib/xmlInput 
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${CMAKE_SOURCE_DIR}/lib/FileIO
  ${CMAKE_SOURCE_DIR}/lib/Calibration
)

#ADD_LIBRARY(Calibration STATIC
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

